#pragma once
#include "Singleton.h"
#include <boost/asio.hpp>
#include <vector>
class AsioIOServicePool : public Singleton<AsioIOServicePool>
{
    friend Singleton<AsioIOServicePool>;

public:
    using IOservice = boost::asio::io_context;
    // This ensures that the io_context's run() and run_one() functions do not exit while the work is underway.
    using Work = boost::asio::io_context::work;
    using WorkPtr = std::unique_ptr<Work>;
    ~AsioIOServicePool();
    AsioIOServicePool(const AsioIOServicePool &) = delete;
    AsioIOServicePool &operator=(const AsioIOServicePool &) = delete;
    // using round-robin method to return an io_context;
    boost::asio::io_context & GetIOService();
    void Stop();

private:
    // Returns the number of concurrent threads supported by the implementation.
    AsioIOServicePool(std::size_t size = std::thread::hardware_concurrency());
    std::vector<IOservice> _ioServices;
    std::vector<WorkPtr> _workers;
    std::vector<std::thread> _threads;
    std::size_t _nextIOService;
};
