
#include "Session.h"
#include "Server.h"
#include <iostream>
#include <iomanip>
#include <json/json.h>
#include <json/value.h>
#include <json/reader.h>

CSession::CSession(boost::asio::io_context &io_context, CServer *server) : _socket(io_context), _server(server), _b_close(false), _b_head_parse(false)
{
    boost::uuids::uuid a_uuid = boost::uuids::random_generator()();
    _uuid = boost::uuids::to_string(a_uuid);
    _recv_head_node = make_shared<MsgNode>(HEAD_TOTAL_LEN);
}
CSession::~CSession()
{
    cout << "~CSession destruct" << endl;
}

tcp::socket &CSession::GetSocket()
{
    return _socket;
}

std::string &CSession::GetUuid()
{
    return _uuid;
}

void CSession::Start()
{
    ::memset(_data, 0, MAX_LENGTH);
    _socket.async_read_some(boost::asio::buffer(_data, MAX_LENGTH), std::bind(&CSession::HandleRead, this,
                                                                              std::placeholders::_1, std::placeholders::_2, SharedSelf()));
}

// Print raw data
void CSession::PrintRecvData(char *data, int length)
{
    stringstream ss;
    string result = "0x";
    for (int i = 0; i < length; i++)
    {
        string hexstr;
        ss << hex << std::setw(2) << std::setfill('0') << int(data[i]) << endl;
        ss >> hexstr;
        result += hexstr;
    }

    std::cout << "receive raw data is : " << result << endl;
}

void CSession::Send(char *msg, int max_length, short msg_id)
{
    bool pending = false;
    std::lock_guard<std::mutex> lock(_send_lock);
    auto send_queue_size = _send_que.size();

    // To avoid the send queue size takes up so much resource
    // we discard the this data/packet
    if (send_queue_size > MAX_SENDQUEUE)
    {
        cout << "session: " << _uuid << " send queue fulled, size is : " << MAX_SENDQUEUE << endl;
        return;
    }

    _send_que.push(make_shared<SendNode>(msg, max_length, msg_id));
    if (send_queue_size > 0)
    {
        return;
    }
    auto &msgnode = _send_que.front();
    boost::asio::async_write(_socket, boost::asio::buffer(msgnode->_data, msgnode->_total_len),
                             std::bind(&CSession::HandleWrite, this, std::placeholders::_1, SharedSelf()));
}

// handle the string message
void CSession::Send(std::string msg, short msg_id)
{
    bool pending = false;
    std::lock_guard<std::mutex> lock(_send_lock);
    auto send_queue_size = _send_que.size();

    // To avoid the send queue size takes up so much resource
    // we discard the this data/packet
    if (send_queue_size > MAX_SENDQUEUE)
    {
        cout << "session: " << _uuid << " send queue fulled, size is : " << MAX_SENDQUEUE << endl;
        return;
    }

    _send_que.push(make_shared<SendNode>(msg.c_str(), msg.length(), msg_id));
    if (send_queue_size > 0)
    {
        return;
    }
    auto &msgnode = _send_que.front();
    boost::asio::async_write(_socket, boost::asio::buffer(msgnode->_data, msgnode->_total_len),
                             std::bind(&CSession::HandleWrite, this, std::placeholders::_1, SharedSelf()));
}

void CSession::Close()
{
    _socket.close();
    _b_close = true;
}

std::shared_ptr<CSession> CSession::SharedSelf()
{
    return shared_from_this();
}

void CSession::HandleWrite(const boost::system::error_code &error, std::shared_ptr<CSession> shared_self)
{

    if (!error)
    {

        std::lock_guard<std::mutex> lock(_send_lock);
        cout << "send data " << _send_que.front()->_data + HEAD_LENGTH << endl;
        _send_que.pop();
        if (!_send_que.empty())
        {
            auto &msgnode = _send_que.front();
            boost::asio::async_write(_socket, boost::asio::buffer(msgnode->_data, msgnode->_total_len),
                                     std::bind(&CSession::HandleWrite, this, std::placeholders::_1, shared_self));
        }
    }
    else
    {
        std::cout << "handle write failed, error is " << error.what() << endl;
        Close();
        _server->clear_session(_uuid);
        // _server->clear_session(_uuid);
    }
}

void CSession::HandleRead(const boost::system::error_code &error, size_t bytes_transferred, std::shared_ptr<CSession> shared_self)
{
    if (!error)
    {
        // number of bytes that have already handled
        //  We reduce the accept rate of the server to test data fragmentation handle operation
        PrintRecvData(_data, bytes_transferred);
        std::chrono::milliseconds dura(2000);
        std::this_thread::sleep_for(dura);

        int copy_len = 0;
        while (bytes_transferred > 0)
        {
            if (!_b_head_parse)
            {
                //
                if (bytes_transferred + _recv_head_node->_cur_len < HEAD_TOTAL_LEN)
                {
                    memcpy(_recv_head_node->_data + _recv_head_node->_cur_len, _data + copy_len, bytes_transferred);
                    _recv_head_node->_cur_len += bytes_transferred;
                    ::memset(_data, 0, MAX_LENGTH);
                    _socket.async_read_some(boost::asio::buffer(_data, MAX_LENGTH),
                                            std::bind(&CSession::HandleRead, this, std::placeholders::_1, std::placeholders::_2, shared_self));
                    return;
                }

                // data receive larger than the header
                // copy the reminder header content into the header receive node.
                int head_remain = HEAD_TOTAL_LEN - _recv_head_node->_cur_len;
                memcpy(_recv_head_node->_data + _recv_head_node->_cur_len, _data + copy_len, head_remain);
                copy_len += head_remain;
                bytes_transferred -= head_remain;

                // GET msg id
                short msg_id = 0;
                memcpy(&msg_id, _recv_head_node->_data, HEAD_ID_LEN);
                // convert msg_id into the host byte order
                msg_id = boost::asio::detail::socket_ops::network_to_host_short(msg_id);
                std::cout << "msg id is : " << msg_id << std::endl;

                // If the msg_id is not valid
                if (msg_id > MAX_LENGTH)
                {
                    std::cout << "invalid msg_id " << msg_id << std::endl;
                    _server->clear_session(_uuid);
                    return;
                }

                // GET HEAD length value
                short msg_len = 0;
                memcpy(&msg_len, _recv_head_node->_data + HEAD_ID_LEN, HEAD_DATA_LEN);
                // Convert the network byte-order into the host byte-order
                msg_len = boost::asio::detail::socket_ops::network_to_host_short(msg_len);
                cout << "msg_len is " << msg_len << endl;

                // if the data length exceeds the size of the buffer
                if (msg_len > MAX_LENGTH)
                {
                    std::cout << "invalid data length is " << msg_len << endl;
                    _server->clear_session(_uuid);
                    return;
                }

                _recv_msg_node = make_shared<ReciveNode>(msg_len, msg_id);

                // indicates that there are still some data did not received
                if (bytes_transferred < msg_len)
                {
                    memcpy(_recv_msg_node->_data + _recv_msg_node->_cur_len, _data + copy_len, bytes_transferred);
                    _recv_msg_node->_cur_len += bytes_transferred;
                    ::memset(_data, 0, MAX_LENGTH);
                    _socket.async_read_some(boost::asio::buffer(_data, MAX_LENGTH),
                                            std::bind(&CSession::HandleRead, this, std::placeholders::_1, std::placeholders::_2, shared_self));
                    _b_head_parse = true;
                    return;
                }

                // indicates that all the data has been received
                memcpy(_recv_msg_node->_data + _recv_msg_node->_cur_len, _data + copy_len, msg_len);
                _recv_msg_node->_cur_len += msg_len;
                copy_len += msg_len;
                bytes_transferred -= msg_len;
                _recv_msg_node->_data[_recv_msg_node->_total_len] = '\0';
                // cout << "receive data is " << _recv_msg_node->_data << endl;
                Json::Reader reader;
                Json::Value root;
                reader.parse(std::string(_recv_msg_node->_data, _recv_msg_node->_total_len), root);
                std::cout << "receiving msg is " << root["id"].asInt() << std::endl;
                std::cout << "msg data is " << root["data"].asString() << std::endl;
                root["data"] = "Server has received msg , msg is " + root["data"].asString();
                std::string return_str = root.toStyledString();

                // Send(_recv_msg_node->_data, _recv_msg_node->_total_len);
                Send(return_str, root["id"].asInt());
                _b_head_parse = false;
                _recv_head_node->Clear();
                if (bytes_transferred <= 0)
                {
                    ::memset(_data, 0, MAX_LENGTH);
                    _socket.async_read_some(boost::asio::buffer(_data, MAX_LENGTH),
                                            std::bind(&CSession::HandleRead, this, std::placeholders::_1, std::placeholders::_2, shared_self));
                    return;
                }
                continue;
            }

            int remain_msg = _recv_msg_node->_total_len - _recv_msg_node->_cur_len;
            if (bytes_transferred < remain_msg)
            {
                memcpy(_recv_msg_node->_data + _recv_msg_node->_cur_len, _data + copy_len, bytes_transferred);
                _recv_msg_node->_cur_len += bytes_transferred;
                ::memset(_data, 0, MAX_LENGTH);
                _socket.async_read_some(boost::asio::buffer(_data, MAX_LENGTH),
                                        std::bind(&CSession::HandleRead, this, std::placeholders::_1, std::placeholders::_2, shared_self));
                return;
            }
            memcpy(_recv_msg_node->_data + _recv_msg_node->_cur_len, _data + copy_len, remain_msg);
            _recv_msg_node->_cur_len += remain_msg;
            bytes_transferred -= remain_msg;
            copy_len += remain_msg;
            _recv_msg_node->_data[_recv_msg_node->_total_len] = '\0';
            // cout << "receive data is " << _recv_msg_node->_data << endl;

            // Send(_recv_msg_node->_data, _recv_msg_node->_total_len);

            Json::Reader reader;
            Json::Value root;
            reader.parse(std::string(_recv_msg_node->_data, _recv_msg_node->_total_len), root);
            std::cout << "receiving msg id is " << root["id"].asInt() << std::endl;
            std::cout << "msg data is " << root["data"].asString() << std::endl;
            root["data"] = "Server has received msg , msg is " + root["data"].asString();
            std::string return_str = root.toStyledString();
            Send(return_str, root["id"].asInt());
            _b_head_parse = false;
            _recv_head_node->Clear();
            if (bytes_transferred <= 0)
            {
                ::memset(_data, 0, MAX_LENGTH);
                _socket.async_read_some(boost::asio::buffer(_data, MAX_LENGTH),
                                        std::bind(&CSession::HandleRead, this, std::placeholders::_1, std::placeholders::_2, shared_self));
                return;
            }
            continue;
        }
    }
    else
    {
        std::cout << "handle read failed, error is " << error.what() << endl;
        Close();
        _server->clear_session(_uuid);
    }
}
