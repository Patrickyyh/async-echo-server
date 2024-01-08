#include <iostream>
// #include <boost/asio.hpp>

#include <thread>
#include <csignal>
#include <mutex>
bool is_server_terminated = false;
std::mutex mutex_terminated;
std::condition_variable cond_terminated;

int main()
{
    std::cout << std::thread::hardware_concurrency() << std::endl;
}
