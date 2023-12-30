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

class Session:public std::enable_shared_from_this<Session>{
    public:
        
        Session(boost::asio::io_context& ioc, Server * server);
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
        void handle_read(const boost::system::error_code & error, size_t bytes_transferred, std::shared_ptr<Session> _self_shared);

        //write data back to the client (callback function)
        void handle_write(const boost::system::error_code & error, std::shared_ptr<Session> _self_shared);
        Server * _server;
        std::string _uuid;

};


