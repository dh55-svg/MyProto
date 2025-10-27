#ifndef __MY_PROTO_SERVER_H
#define __MY_PROTO_SERVER_H

#include <memory>
#include "muduo/net/TcpServer.h"
#include "ConnectionHandler.h"
#include "BusinessHandler.h"

class MyProtoServer {
public:
    using EventLoop = muduo::net::EventLoop;
    
    MyProtoServer(EventLoop* loop, const muduo::net::InetAddress& listenAddr, const std::string& nameArg);
    ~MyProtoServer();
    
    void start();
    void stop();
    
    // 获取业务处理器，用于注册业务逻辑
    std::shared_ptr<BusinessHandler> getBusinessHandler();
    
private:
    muduo::net::TcpServer server_;
    std::shared_ptr<ConnectionHandler> connectionHandler_;
    std::shared_ptr<BusinessHandler> businessHandler_;
    
    // 定时器回调，用于检查超时消息
    void onTimeout();
};

#endif // __MY_PROTO_SERVER_H