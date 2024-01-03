#pragma once
#include <memory>
#include <mutex>
#include <iostream>
template<typename T>
class Singleton{
    private:
    Singleton()=default;
    Singleton(const Singleton<T>&) = delete;
    Singleton & operator = (const Singleton<T> & st) = delete;

    static std::shared_ptr<T> _instance;
    public:
    ~Singleton(){
        std::cout << "this is singleton destruct" << std::endl;
    }

    


};
