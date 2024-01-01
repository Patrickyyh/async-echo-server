#include <iostream>
#include <boost/asio.hpp>
#include "Server.h"
int main(){
    try
    {
       boost::asio::io_context ioc;
       using namespace std;
       CServer server(ioc,10086);
       ioc.run();
       

    }
    catch(const std::exception& e)
    {
        std::cerr << e.what() << '\n';
    }

}
