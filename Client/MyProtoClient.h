#ifndef __MY_PROTO_CLIENT_H
#define __MY_PROTO_CLIENT_H

#include <memory>
#include "muduo/net/TcpClient.h"
#include "muduo/net/TimerId.h" // 添加TimerId头文件
#include "ConnectionHandler.h"

class MyProtoClient {
public:
    using EventLoop = muduo::net::EventLoop;
    using TcpConnectionPtr = muduo::net::TcpConnectionPtr;
    using MessageCallback = ConnectionHandler::MessageCallback;
    
    MyProtoClient(EventLoop* loop, const muduo::net::InetAddress& serverAddr, const std::string& nameArg);
    ~MyProtoClient();
    
    void connect();
    void disconnect();
    void stop();
    
    // 发送消息
    uint32_t sendMessage(const MyProtoMsg& msg);
    
    // 设置消息回调
    void setMessageCallback(const MessageCallback& cb);
    
    // 获取连接状态
    bool isConnected() const;
    
    // 设置重连参数
    void setReconnectInterval(int intervalMs);
    void enableAutoReconnect(bool enable);

private:
    // 定时器回调函数
    void onTimeout();
    
    // 处理连接断开的方法
    // 修改方法声明
    void handleConnectionClosed(const TcpConnectionPtr& conn);
    
    muduo::net::TcpClient client_;
    muduo::net::InetAddress serverAddr_; // 添加服务器地址成员变量
    std::shared_ptr<ConnectionHandler> connectionHandler_;
    MessageCallback messageCallback_;
    muduo::net::TimerId timerId_; // 定时器ID
    muduo::net::TimerId reconnectTimerId_; // 重连定时器ID
    int reconnectIntervalMs_; // 重连间隔（毫秒）
    bool autoReconnect_; // 是否启用自动重连
};

#endif // __MY_PROTO_CLIENT_H