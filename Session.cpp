#include "Session.h"
#include <iostream>
using namespace std;
void Session::Start(){
    //clear the buffer
    memset(this->_data ,  0, max_length);
    _socket.async_read_some(boost::asio::buffer(_data , max_length), 
                            std::bind(&Session::handle_read , this , placeholders::_1 , placeholders::_2));
                        
}

void Session::handle_read(const boost::system::error_code & error, size_t bytes_transferred){
    
    if(!error){
        cout << "server received message: " << _data << endl;
        boost::asio::async_write(_socket, boost::asio::buffer(_data, bytes_transferred), 
            std::bind(&Session::handle_write, this, placeholders::_1));
    }else{
        cout << "read error" << endl;
        //delete the session
        delete this;
    }
}

void Session::handle_write(const boost::system::error_code & error){
      if(!error){
    
        memset(this->_data , 0 , max_length);
        _socket.async_read_some(boost::asio::buffer(_data , max_length), 
                            std::bind(&Session::handle_read , this , placeholders::_1 , placeholders::_2));

    }else{
        cout << "write error" << endl;
        //delete the session
        delete this;
    }
}

/*
_acceptor(ioc , tcp::endpoint(tcp::v4() , port)
- This step basically create an endpoint based on a random v4 ipaddress on the local machine and a port
- The acceptor will accept a connection from clinet to this endpoint
*/
 Server::Server(boost::asio::io_context & ioc , short port):_ioc(ioc) 
 ,_acceptor(ioc , tcp::endpoint(tcp::v4() , port)){
    cout << "Server start success, on port: " << port << endl;
    start_accept();
 }

 void Server::start_accept(){
    
    // Create a new session
    Session * new_session = new Session(_ioc);
    
    // accept the connecion with async_accept

    /*
        The socket into which the new connection will be accepted. Ownership of the peer object is retained by the 
        caller,  which must guarantee that it is valid until the completion handler is called
    */
    _acceptor.async_accept(new_session->Socket() , 
                          std::bind(&Server::handle_accept,this,new_session, std::placeholders::_1));

 }

 void Server::handle_accept(Session * new_session , const boost::system::error_code & error){
    
    // handle one accept
    if(!error){
        new_session->Start();
    }else {
        delete new_session;
    }

    // accept new connection after handling one accept
    start_accept();
 }

 

