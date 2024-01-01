/*
_acceptor(ioc , tcp::endpoint(tcp::v4() , port)
- This step basically create an endpoint based on a random v4 ipaddress on the local machine and a port
- The acceptor will accept a connection from clinet to this endpoint
*/
#include "Server.h"
#include <iostream>

Server::Server(boost::asio::io_context &ioc, short port) : _ioc(ioc), _acceptor(ioc, tcp::endpoint(tcp::v4(), port))
{
    cout << "Server start success, on port: " << port << endl;
    start_accept();
}

void Server::clear_session(std::string uuid)
{
    _session_collection.erase(uuid);
}

void Server::start_accept()
{

    // Create a new session
    std::shared_ptr<Session> new_session = std::make_shared<Session>(_ioc, this);

    // accept the connecion with async_accept

    /*
        The socket into which the new connection will be accepted. Ownership of the peer object is retained by the
        caller,  which must guarantee that it is valid until the completion handler is called
    */

    /*
        Notice that since we bind the new_session with the handle_accept function,
        the reference count of the new_session will be incremented.
        1. As long as the bind function is in the event queue of the asio. Its lifetime will be retained.
        2. However, if the handle_accept function has been called by asio and it is removed from the event-queue
        then the function returned by bind will be deallocated, the reference count will also be decremented.
        3. So we could generate an unique session id and add the session with the id into the map of the server.
    */
    _acceptor.async_accept(new_session->Socket(),
                           std::bind(&Server::handle_accept, this, new_session, std::placeholders::_1));
}

void Server::handle_accept(std::shared_ptr<Session> new_session, const boost::system::error_code &error)
{

    // handle one accept
    if (!error)
    {

        new_session->Start();
        // put the session into the map
        _session_collection.insert(std::make_pair(new_session->get_uuid(), new_session));
    }
    else
    {
        // delete new_session;
    }

    // accept new connection after handling one accept
    start_accept();
}
