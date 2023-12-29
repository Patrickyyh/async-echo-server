#pragma once
#include <memory>
#include "boost/asio.hpp"
#include <iostream>
#include <queue>
using namespace boost;
using boost::asio::ip::tcp;
using namespace std;
const int RECVSIZE = 1024;
class Session{
    public:
        Session(boost::asio::io_context& ioc):_socket(ioc){
            
        }

    //Getter function for the socket
    tcp::socket & Socket(){
        return this->_socket;
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

};


class Server{
    public:
     Server(boost::asio::io_context & ioc , short port);

    private:
        // listen and accept the connection from the client
        void start_accept();

        // callback function after the connection has been accepted
        void handle_accept(Session * new_session , const boost::system::error_code & error);

        // Since the io_context could not be copied and only could be referenced 
        boost::asio::io_context & _ioc;
        boost::asio::ip::tcp::acceptor _acceptor;
};