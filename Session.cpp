#include "Session.h"
#include "Server.h"
#include <iostream>
using namespace std;

Session::Session(boost::asio::io_context &ioc, Server *server) : _socket(ioc), _server(server)
{
    // generate an uuid when a session is created
    boost::uuids::uuid session_id = boost::uuids::random_generator()();
    _uuid = boost::uuids::to_string(session_id);
    _recv_head_node = make_shared<MsgNode>(MESSAGE_LENGTH_HEAD_LENGTH);
}

void Session::Send(char *msg, int max_length)
{

    bool pending = false;
    std::lock_guard<std::mutex> lock(_send_lock);

    if (_send_queue.size() > 0)
        pending = true;
    _send_queue.push(make_shared<MsgNode>(msg, max_length));

    if (pending)
        return;
    boost::asio::async_write(_socket, boost::asio::buffer(msg, max_length),
                             std::bind(&Session::handle_write, this, placeholders::_1, shared_from_this()));
}

void Session::Start()
{
    // clear the buffer
    memset(this->_data, 0, max_length);

    // To avoid create different share_ptr points at the same session
    // we could use the enable_shared_from_this to synchronize the reference count
    _socket.async_read_some(boost::asio::buffer(_data, max_length),
                            std::bind(&Session::handle_read, this, placeholders::_1, placeholders::_2, shared_from_this()));
}

void Session::handle_read(const boost::system::error_code &error, size_t bytes_transferred, std::shared_ptr<Session> _self_shared)
{

    // if (!error)
    // {
    //     // cout << "server received message: " << _data << endl;
    //     // // memset(_data, 0 , max_length);
    //     // // _socket.async_read_some(boost::asio::buffer(_data , max_length),
    //     // //                         std::bind(&Session::handle_read , this , placeholders::_1 , placeholders::_2, _self_shared));

    //     // boost::asio::async_write(_socket, boost::asio::buffer(_data, bytes_transferred),
    //     //                          std::bind(&Session::handle_write, this, placeholders::_1, _self_shared));

    //     cout << "server received message: " << _data << endl;
    //     Send(_data, bytes_transferred);
    //     memset(_data, 0, max_length);
    //     _socket.async_read_some(boost::asio::buffer(_data, max_length),
    //                             std::bind(&Session::handle_read, this, placeholders::_1, placeholders::_2, _self_shared));
    // }
    // else
    // {
    //     cout << "read error" << endl;
    //     // delete the session
    //     // delete this;
    //     _server->clear_session(_uuid);
    // }

    if (!error)
    {
        int n_of_bytes_moved = 0;
        while (bytes_transferred > 0)
        {
            if (!if_head_parsed)
            {
                if (_recv_head_node->_current_length + bytes_transferred < MESSAGE_LENGTH_HEAD_LENGTH)
                {
                    // Indicates the data we received is less than the size of header
                    memcpy(_recv_head_node->_data + _recv_head_node->_current_length, _data + n_of_bytes_moved, bytes_transferred);
                    _recv_head_node->_current_length += bytes_transferred;
                    ::memset(_data, 0, MAX_LENGTH);
                    // continue reading data;
                    _socket.async_read_some(boost::asio::buffer(_data, MAX_LENGTH),
                                            std::bind(&Session::handle_read, this, placeholders::_1, placeholders::_2, _self_shared));
                    return;
                }

                // if size of data recevied is larger than or equal to the header section size
                int size_of_head_buffer_reminded = MESSAGE_LENGTH_HEAD_LENGTH - _recv_head_node->_current_length;
                memcpy(_recv_head_node->_data + _recv_head_node->_current_length, _data + n_of_bytes_moved, size_of_head_buffer_reminded);
                n_of_bytes_moved += size_of_head_buffer_reminded;
                bytes_transferred -= size_of_head_buffer_reminded;

                // Access the header value and print it out
                short header_value = 0;
                memcpy(&header_value, _recv_head_node->_data, MESSAGE_LENGTH_HEAD_LENGTH);
                cout << "header value: " << header_value << endl;

                // if the msg's length is larger than the max bytes that buffer can handled
                // we discard this message
                if (header_value > MAX_LENGTH)
                {
                    std::cout << "invalid data length is " << header_value << endl;
                    _server->clear_session(_uuid);
                    return;
                }

                // Continue on copying data to the receive msg node
                _recv_msg_node = make_shared<MsgNode>(header_value);

                // if the data received less than the value in the header
                // this indicates the msg has not sent completely
                if (bytes_transferred < header_value)
                {
                    memcpy(_recv_msg_node->_data + _recv_msg_node->_current_length, _data + n_of_bytes_moved, bytes_transferred);
                    _recv_msg_node->_current_length += bytes_transferred;
                    ::memset(_data, 0, MAX_LENGTH);
                    _socket.async_read_some(boost::asio::buffer(_data, MAX_LENGTH),
                                            std::bind(&Session::handle_read, this, placeholders::_1, placeholders::_2, _self_shared));
                    if_head_parsed = true;
                    return;
                }

                // noraml data copy routine from buffer to the recev_msg_node
                memcpy(_recv_msg_node->_data + _recv_msg_node->_current_length, _data + n_of_bytes_moved, header_value);
                _recv_msg_node->_current_length += header_value;
                n_of_bytes_moved += header_value;
                bytes_transferred -= header_value;
                _recv_msg_node->_data[_recv_msg_node->_max_length] = '\0';
                cout << "Receive msg is : " << _recv_msg_node->_data << endl;

                // Send the msg back to the client for testing
                Send(_recv_msg_node->_data, _recv_msg_node->_max_length);

                // Since it has possibility that client send huge amount of the data
                //  we need to cut the data frame
                if_head_parsed = false;
                _recv_head_node->clear_data();

                if (bytes_transferred <= 0)
                {
                    ::memset(_data, 0, MAX_LENGTH);
                    _socket.async_read_some(boost::asio::buffer(_data, MAX_LENGTH),
                                            std::bind(&Session::handle_read, this, placeholders::_1, placeholders::_2, _self_shared));
                    return;
                }

                continue;
            }

            int remain_msg_size = _recv_msg_node->_max_length - _recv_msg_node->_current_length;
            if (bytes_transferred < remain_msg_size)
            {
                memcpy(_recv_msg_node->_data + _recv_msg_node->_current_length, _data + n_of_bytes_moved, bytes_transferred);
                _recv_msg_node->_current_length += bytes_transferred;
                ::memset(_data, 0, MAX_LENGTH);
                _socket.async_read_some(boost::asio::buffer(_data, MAX_LENGTH),
                                        std::bind(&Session::handle_read, this, placeholders::_1, placeholders::_2, _self_shared));
                return;
            }

            memcpy(_recv_msg_node->_data + _recv_msg_node->_current_length, _data + n_of_bytes_moved, remain_msg_size);
            _recv_msg_node->_current_length += remain_msg_size;
            bytes_transferred -= remain_msg_size;
            n_of_bytes_moved += remain_msg_size;
            _recv_msg_node->_data[_recv_msg_node->_max_length] = '\0';
            cout << "Receive msg: " << _recv_msg_node->_data << endl;
            Send(_recv_msg_node->_data, _recv_msg_node->_max_length);

            if_head_parsed = false;
            _recv_head_node->clear_data();
            if (bytes_transferred <= 0)
            {
                ::memset(_data, 0, MAX_LENGTH);
                _socket.async_read_some(boost::asio::buffer(_data, MAX_LENGTH),
                                        std::bind(&Session::handle_read, this, placeholders::_1, placeholders::_2, _self_shared));
                return;
            }
            continue;
        }
    }
    else
    {
        cout << "read error" << endl;
        //     // delete the session
        //     // delete this;
        _server->clear_session(_uuid);
    }
}

void Session::handle_write(const boost::system::error_code &error, std::shared_ptr<Session> _self_shared)
{
    if (!error)
    {
        std::lock_guard<std::mutex> lock(_send_lock);

        // pop out the MsgNode that has already been sent
        _send_queue.pop();
        if (!_send_queue.empty())
        {
            auto &msg_node = _send_queue.front();
            boost::asio::async_write(_socket, boost::asio::buffer(msg_node->_data, msg_node->_max_length),
                                     std::bind(&Session::handle_write, this, placeholders::_1, _self_shared));
        }

        // memset(this->_data, 0, max_length);
        // _socket.async_read_some(boost::asio::buffer(_data, max_length),
        //                         std::bind(&Session::handle_read, this, placeholders::_1, placeholders::_2, _self_shared));
    }
    else
    {
        cout << "write error" << endl;
        // delete the session
        // delete this;
        _server->clear_session(_uuid);
    }
}
