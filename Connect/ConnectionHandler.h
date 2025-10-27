#ifndef __CONNECTION_HANDLER_H
#define __CONNECTION_HANDLER_H

#include <functional>
#include <memory>
#include <muduo/net/TcpConnection.h>
#include "myproto.h"
#include "ReliableMsgManager.h"

class BusinessHandler;

class ConnectionHandler {
public:
    using TcpConnectionPtr = muduo::net::TcpConnectionPtr;
    using MessageCallback = std::function<void(const TcpConnectionPtr&, std::shared_ptr<MyProtoMsg>)>;
    using ConnectionCallback = std::function<void(const TcpConnectionPtr&)>;
    
    ConnectionHandler();
    ~ConnectionHandler();
    
    void setBusinessHandler(std::shared_ptr<BusinessHandler> handler);
    void setMessageCallback(const MessageCallback& cb);
    
    // 设置连接回调
    void setConnectionCallback(const ConnectionCallback& cb);
    
    // 连接回调函数
    void onConnection(const TcpConnectionPtr& conn);
    void onMessage(const TcpConnectionPtr& conn, muduo::net::Buffer* buf, muduo::Timestamp time);
    void onWriteComplete(const TcpConnectionPtr& conn);
    
    // 发送消息方法
    uint32_t sendMessage(const TcpConnectionPtr& conn, const MyProtoMsg& msg);
    
    // 定期检查超时消息
    void checkTimeoutMessages();
    
    // 在private部分添加connectionCallback_成员变量
    private:
        MyProtoDecode decoder_; // 协议解码器
        ReliableMsgManager reliableManager_; // 可靠消息管理器
        std::shared_ptr<BusinessHandler> businessHandler_; // 业务处理器
        MessageCallback messageCallback_; // 消息回调
        ConnectionCallback connectionCallback_; // 连接回调
};

#endif // __CONNECTION_HANDLER_H