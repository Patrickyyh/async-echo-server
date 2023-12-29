#pragma once
#include <memory>
#include "boost/asio.hpp"
#include <iostream>
#include <queue>
#include <memory>
#include "boost/uuid/uuid_generators.hpp"
#include "boost/uuid/uuid_io.hpp"

using namespace boost;
using boost::asio::ip::tcp;
using namespace std;
const int RECVSIZE = 1024;
class Server; 

class Session{
    public:
        Session(boost::asio::io_context& ioc, Server * server):_socket(ioc),_server(server){
            //generate an uuid when a session is created
            boost::uuids::uuid session_id = boost::uuids::random_generator()();
            _uuid =boost::uuids::to_string(session_id);
        }

    //Getter function for the socket
    tcp::socket & Socket(){
        return this->_socket;
    }

    //Getter function for the uuid
    std::string & get_uuid(){
        return this->_uuid;
    }

    /**
     * Monitor if the client sent the message to the server
    */
    void Start();


    private:
        // responsible for handling the read and write operation 
        tcp::socket _socket;
        enum {max_length = 1024};

        // store the data received from the client 
        char _data[max_length];
        
        //read the data from the client(callback function)
        void handle_read(const boost::system::error_code & error, size_t bytes_transferred);

        //write data back to the client (callback function)
        void handle_write(const boost::system::error_code & error);
        Server * _server;
        std::string _uuid;

};


class Server{
    public:
     Server(boost::asio::io_context & ioc , short port);

    private:
        // listen and accept the connection from the client
        void start_accept();

        // callback function after the connection has been accepted
        void handle_accept(std::shared_ptr<Session> new_session , const boost::system::error_code & error);

        // Since the io_context could not be copied and only could be referenced 
        boost::asio::io_context & _ioc;
        boost::asio::ip::tcp::acceptor _acceptor;

        // management of the session ptr
        std::map<std::string, std::shared_ptr<Session>> _session_collection;

};