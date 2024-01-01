#pragma once
#include <memory>
#include "boost/asio.hpp"
#include <iostream>
#include <queue>
#include <memory>
#include <mutex>
#include "boost/uuid/uuid_generators.hpp"
#include "boost/uuid/uuid_io.hpp"
#define MESSAGE_LENGTH_HEAD_LENGTH 2
#define MAX_LENGTH 2048


using namespace boost;
using boost::asio::ip::tcp;
using namespace std;
const int RECVSIZE = 1024;
class Server;

class MsgNode
{
    friend class Session;

public:
    MsgNode(char *msg, int max_length) : _max_length(max_length + MESSAGE_LENGTH_HEAD_LENGTH),
                                         _current_length(0)
    {
        _data = new char[max_length + 1]();
        memcpy(_data, &max_length, MESSAGE_LENGTH_HEAD_LENGTH);
        memcpy(_data + MESSAGE_LENGTH_HEAD_LENGTH, msg, max_length);
        _data[_max_length] = '\0';
    }

    MsgNode(short max_len) : _max_length(max_len), _current_length(0)
    {
        _data = new char[_max_length + 1]();
    }

    ~MsgNode()
    {
        delete[] _data;
    }

    void clear_data()
    {
        // clear the data over here
        memset(_data, 0, _max_length);
        _current_length = 0;
    }

private:
    int _current_length;
    int _max_length;
    char *_data;
};

class Session : public std::enable_shared_from_this<Session>
{
public:
    Session(boost::asio::io_context &ioc, Server *server);
    // Getter function for the socket
    tcp::socket &Socket()
    {
        return this->_socket;
    }

    // Getter function for the uuid
    std::string &get_uuid()
    {
        return this->_uuid;
    }

    /**
     * Monitor if the client sent the message to the server
     */
    void Start();

    void Send(char *msg, int max_length);

private:
    // responsible for handling the read and write operation
    tcp::socket _socket;
    enum
    {
        max_length = 1024
    };

    // store the data received from the client
    char _data[max_length];

    // read the data from the client(callback function)
    void handle_read(const boost::system::error_code &error, size_t bytes_transferred, std::shared_ptr<Session> _self_shared);
    // write data back to the client (callback function)
    void handle_write(const boost::system::error_code &error, std::shared_ptr<Session> _self_shared);
    Server *_server;
    std::string _uuid;
    std::queue<std::shared_ptr<MsgNode>> _send_queue;
    std::mutex _send_lock;

    // data-structure for Msg
    // To store the the msgNode received by the server
    std::shared_ptr<MsgNode> _recv_msg_node;
    // To store the head
    std::shared_ptr<MsgNode> _recv_head_node;
    bool if_head_parsed;

};
