#include "LogicSystem.h"
using namespace std;

LogicSystem::LogicSystem() : _b_stop(false)
{

    RegisterCallBacks();
    _worker_thread = std::thread(&LogicSystem::DealMsg, this);
}

void LogicSystem::RegisterCallBacks()
{
    // put the
    _function_call_back[MSG_HELLO_WORD] = std::bind(&LogicSystem::HelloWorldCallBack, this, placeholders::_1,
                                                    placeholders::_2, placeholders::_3);
}

void LogicSystem::HelloWorldCallBack(shared_ptr<CSession> session, const short &msg_id, const string &msg_data)
{
    Json::Reader reader;
    Json::Value root;
    reader.parse(msg_data, root);
    std::cout << "receive msg id: " << root["id"].asInt() << std::endl;
    std::cout << "receive data : " << root["data"].asString() << std::endl;
    root["data"] = "Server has received msg, msg data is " + root["data"].asString();
    std::string return_str = root.toStyledString();
    session->Send(return_str, root["id"].asInt());
}

void LogicSystem::DealMsg()
{
    for (;;)
    {
        std::unique_lock<std::mutex> unique_lk(_mutex);
        // when the queue is empty
        // give up the CPU and unlock the mutex;
        while (_msg_queue.empty() && !_b_stop)
        {
            _consume.wait(unique_lk);
        }

        // if _b_stop == true. get all the data from the _msg_queue, handle msg and break the while loop
        if (_b_stop)
        {
            while (!_msg_queue.empty())
            {
                auto msg_node = _msg_queue.front();
                cout << "recv msg id is " << msg_node->_recvnode->_msg_id << endl;

                // find a callback function
                auto call_back_iter = _function_call_back.find(msg_node->_recvnode->_msg_id);
                if (call_back_iter == _function_call_back.end())
                {
                    _msg_queue.pop();
                    continue;
                }

                call_back_iter->second(msg_node->_session, msg_node->_recvnode->_msg_id,
                                       std::string(msg_node->_recvnode->_data, msg_node->_recvnode->_cur_len));
                _msg_queue.pop();
            }
            break;
        }

        // if _b_stop = False ,
        auto msg_node = _msg_queue.front();
        cout << "recv msg id is " << msg_node->_recvnode->_msg_id << endl;
        auto call_back_iter = _function_call_back.find(msg_node->_recvnode->_msg_id);
        if (call_back_iter == _function_call_back.end())
        {
            _msg_queue.pop();
            continue;
        }
        call_back_iter->second(msg_node->_session, msg_node->_recvnode->_msg_id,
                               std::string(msg_node->_recvnode->_data, msg_node->_recvnode->_cur_len));
        _msg_queue.pop();
    }
}

void LogicSystem::PostMsgtoQueue(shared_ptr<LogicNode> msg)
{
    std::unique_lock<std::mutex> unique_lk(_mutex);
    _msg_queue.push(msg);
    if (_msg_queue.size() == 1)
    {
        _consume.notify_one();
    }
}

LogicSystem::~LogicSystem()
{
    _b_stop = true;
    _consume.notify_one();
    _worker_thread.join();
}
