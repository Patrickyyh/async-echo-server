#include "Session.h"
#include "Server.h"
#include <iostream>
using namespace std;

Session::Session(boost::asio::io_context &ioc, Server *server) : _socket(ioc), _server(server)
{
    // generate an uuid when a session is created
    boost::uuids::uuid session_id = boost::uuids::random_generator()();
    _uuid = boost::uuids::to_string(session_id);
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

    if (!error)
    {
        // cout << "server received message: " << _data << endl;
        // // memset(_data, 0 , max_length);
        // // _socket.async_read_some(boost::asio::buffer(_data , max_length),
        // //                         std::bind(&Session::handle_read , this , placeholders::_1 , placeholders::_2, _self_shared));

        // boost::asio::async_write(_socket, boost::asio::buffer(_data, bytes_transferred),
        //                          std::bind(&Session::handle_write, this, placeholders::_1, _self_shared));

        cout << "server received message: " << _data << endl;
        Send(_data, bytes_transferred);
        memset(_data, 0, max_length);
        _socket.async_read_some(boost::asio::buffer(_data, max_length),
                                std::bind(&Session::handle_read, this, placeholders::_1, placeholders::_2, _self_shared));
    }
    else
    {
        cout << "read error" << endl;
        // delete the session
        // delete this;
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
