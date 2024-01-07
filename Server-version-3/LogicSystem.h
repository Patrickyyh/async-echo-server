#pragma once
#include "Singleton.h"
#include <queue>
#include <thread>
#include "Session.h"
#include <map>
#include <functional>
#include "const.h"
#include <json/json.h>
#include <json/value.h>
#include <json/reader.h>

typedef function<void(shared_ptr<CSession>, const short &msg_id, const string &msg_data)> FunctionCallBack;

class LogicSystem : public Singleton<LogicSystem>
{
    friend class Singleton<LogicSystem>;

public:
    ~LogicSystem();

    // send the data and session into the logic queue
    // called from the asio layer
    void PostMsgtoQueue(shared_ptr<LogicNode> msg);

private:
    LogicSystem();
    void RegisterCallBacks();
    void HelloWorldCallBack(shared_ptr<CSession> session, const short &msg_id, const string &msg_data);
    void DealMsg();

    std::queue<shared_ptr<LogicNode>> _msg_queue;

    // Since both the logic layer and asio layer will access the msg_queue
    // So we need to add the lock at here
    std::mutex _mutex;
    // Since both consumer and producer need to update the msg_queue
    std::condition_variable _consume;

    // Worker thread get data and callback from the logic queue and
    // execute the call back function.
    std::thread _worker_thread;
    bool _b_stop;
    // bind the message_id and its callback function
    std::map<short, FunctionCallBack> _function_call_back;
};
