#include <boost/asio.hpp>
#include "Session.h"
#include <memory.h>
#include <map>

class Server{
    public:
     Server(boost::asio::io_context & ioc , short port);

     // cleare the session
     void clear_session(std::string uuid);

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