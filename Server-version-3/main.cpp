#include <iostream>
#include <boost/asio.hpp>
#include "Server.h"
#include <thread>
#include <csignal>
#include <mutex>
bool is_server_terminated = false;
std::mutex mutex_terminated;
std::condition_variable cond_terminated;

void signal_handler(int sig)
{
    if (sig == SIGINT || sig == SIGTERM)
    {
        std::unique_lock<std::mutex> lock_quit(mutex_terminated);
        is_server_terminated = true;
        std::cout <<" executed at here " << std::endl;
        cond_terminated.notify_one();
    }
}

int main()
{
    try
    {
        boost::asio::io_context ioc;
        std::thread network_thread([&ioc]
                                   {
            CServer server(ioc, 10086);
            ioc.run(); });


        signal(SIGINT , signal_handler);
        signal(SIGTERM , signal_handler);
        while (!is_server_terminated)
        {
            std::unique_lock<std::mutex> lock_quit(mutex_terminated);
            cond_terminated.wait(lock_quit);
        }

        ioc.stop();
        network_thread.join();
    }
    catch (const std::exception &e)
    {
        std::cerr << e.what() << '\n';
    }
}
