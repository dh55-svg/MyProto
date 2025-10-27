#ifndef __BUSINESS_HANDLER_H
#define __BUSINESS_HANDLER_H

#include <memory>
#include <unordered_map>
#include <functional>
#include <muduo/net/TcpConnection.h>
#include "../Myproto/myproto.h"

class ConnectionHandler;

class BusinessHandler {
public:
    using TcpConnectionPtr = muduo::net::TcpConnectionPtr;
    using MessageHandler = std::function<void(const TcpConnectionPtr&, const std::shared_ptr<MyProtoMsg>&, ConnectionHandler*)>;
    
    BusinessHandler();
    ~BusinessHandler();
    
    void setConnectionHandler(std::shared_ptr<ConnectionHandler> handler);
    
    // 注册业务处理函数
    void registerHandler(uint16_t serverId, const MessageHandler& handler);
    
    // 处理消息入口
    void handleMessage(const TcpConnectionPtr& conn, const std::shared_ptr<MyProtoMsg>& msg);
    
    // 发送响应消息
    uint32_t sendResponse(const TcpConnectionPtr& conn, uint16_t serverId, const json& responseBody);
    
private:
    std::shared_ptr<ConnectionHandler> connectionHandler_;
    //消息路由机制
    /*
        param serverId: 服务器ID，用于标识不同的业务处理模块
        param handler: 消息处理函数，符合MessageHandler类型定义的回调函数
    */
    std::unordered_map<uint16_t, MessageHandler> messageHandlers_;
};

#endif // __BUSINESS_HANDLER_H