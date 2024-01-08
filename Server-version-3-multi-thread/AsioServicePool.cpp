#include "AsioServicePool.h"
AsioIOServicePool::AsioIOServicePool(std::size_t size) : _ioServices(size), _workers(size), _nextIOService(0)
{
    for (std::size_t i = 0; i < size; i++)
    {
        _workers[i] = std::unique_ptr<Work>(new Work(_ioServices[i]));
    }

    // Iterate all the Ioservice, create threads and run the ioservice inside the thread
    for (std::size_t i = 0; i < _ioServices.size(); i++)
    {
        // avoid thread copy to reduce another constructor
        _threads.emplace_back([this, i]()
                              { _ioServices[i].run(); });
    }
}

AsioIOServicePool::~AsioIOServicePool()
{
    std::cout << "AsioIoService destruct" << std::endl;
}

boost::asio::io_context &AsioIOServicePool::GetIOService()
{
    auto &service = _ioServices[_nextIOService++];
    if (_nextIOService == _ioServices.size())
    {
        _nextIOService = 0;
    }

    return service;
}

void AsioIOServicePool::Stop()
{
    for (auto &worker : _workers)
    {
        worker.reset();
    }

    for (auto &t : _threads)
    {
        t.join();
    }
}


