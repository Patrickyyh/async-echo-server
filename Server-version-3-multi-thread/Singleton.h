#pragma once
#include <memory>
#include <mutex>
#include <iostream>
using namespace std;
template <typename T>
class Singleton
{
protected:
    Singleton() = default;
    Singleton(const Singleton<T> &) = delete;
    Singleton &operator=(const Singleton<T> &st) = delete;
    static std::shared_ptr<T> _instance;

public:
    ~Singleton()
    {
        std::cout << "this is singleton destruct" << std::endl;
    }

    static std::shared_ptr<T> GetInstance()
    {
        static std::once_flag s_flag;

        // Since this function could only called once
        //  Hence it is thread-safe
        std::call_once(s_flag, [&]()
                       { _instance = shared_ptr<T>(new T); });

            return _instance;
    }
};

template <typename T>
std::shared_ptr<T> Singleton<T>::_instance = nullptr;
