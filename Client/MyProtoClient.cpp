#include <iostream>
#include "MyProtoClient.h"
#include "muduo/net/EventLoop.h"

// 修改MyProtoClient构造函数中的回调设置部分
MyProtoClient::MyProtoClient(EventLoop* loop, const muduo::net::InetAddress& serverAddr, const std::string& nameArg)
    : client_(loop, serverAddr, nameArg),
      serverAddr_(serverAddr),
      connectionHandler_(new ConnectionHandler()),
      reconnectIntervalMs_(3000),
      autoReconnect_(true) {
    
    std::cout << "[Client] Constructor: Creating connectionHandler_" << std::endl;
    
    // 先设置ConnectionHandler的回调
    std::cout << "[Client] Setting connection callback on handler" << std::endl;
    connectionHandler_->setConnectionCallback(
        std::bind(&MyProtoClient::handleConnectionClosed, this, std::placeholders::_1)
    );
    
    // 然后再设置TcpClient的回调
    std::cout << "[Client] Setting TcpClient connection callback" << std::endl;
    client_.setConnectionCallback(
        std::bind(&ConnectionHandler::onConnection, connectionHandler_, std::placeholders::_1)
    );
    
    std::cout << "[Client] Setting TcpClient message callback" << std::endl;
    client_.setMessageCallback(
        std::bind(&ConnectionHandler::onMessage, connectionHandler_, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3)
    );
    
    // 保存定时器ID
    timerId_ = loop->runEvery(0.5, std::bind(&MyProtoClient::onTimeout, this));
}

// 修改析构函数中的TimerId处理
MyProtoClient::~MyProtoClient() {
    // 先取消定时器
    client_.getLoop()->cancel(timerId_);
    // 直接调用cancel，不进行比较运算
    client_.getLoop()->cancel(reconnectTimerId_);
    // 再断开连接
    stop();
}

// 修改connect方法
void MyProtoClient::connect() {
   std::cout << "[Client] Connecting to server at " 
              << serverAddr_.toIpPort() << "..." << std::endl;
    client_.connect();
}

// 修改isConnected方法，移除对私有成员state_的访问
bool MyProtoClient::isConnected() const {
    bool connected = client_.connection() && client_.connection()->connected();
    std::cout << "[Client] Connection status: " 
              << (connected ? "connected" : "disconnected") << std::endl;
    
    if (!connected && client_.connection()) {
        std::cout << "[Client] Connection exists but not connected, state: unavailable (private member)" << std::endl;
    }
    
    return connected;
}

// 修改handleConnectionClosed方法
void MyProtoClient::handleConnectionClosed(const TcpConnectionPtr& conn) {
    std::cout << "[Client] handleConnectionClosed called, connection name: " 
              << conn->name() << ", connected: " << (conn->connected() ? "yes" : "no") << std::endl;
    
    // 只在连接断开时处理
    if (!conn->connected() && autoReconnect_ && !client_.connection()) {
        std::cout << "[Client] Connection closed, scheduling reconnection in " 
                  << reconnectIntervalMs_ << "ms" << std::endl;
        
        // 取消之前的重连定时器，直接调用cancel
        client_.getLoop()->cancel(reconnectTimerId_);
        
        // 设置新的重连定时器
        reconnectTimerId_ = client_.getLoop()->runAfter(
            reconnectIntervalMs_ / 1000.0,
            std::bind(&MyProtoClient::connect, this)
        );
    } else if (conn->connected()) {
        std::cout << "[Client] Connection established, no need for reconnection" << std::endl;
    } else if (!autoReconnect_) {
        std::cout << "[Client] Auto-reconnect is disabled" << std::endl;
    }
}

void MyProtoClient::disconnect() {
    client_.disconnect();
}

void MyProtoClient::stop() {
    client_.getLoop()->queueInLoop([this]() {
        client_.disconnect();
    });
}

uint32_t MyProtoClient::sendMessage(const MyProtoMsg& msg) {
    if (!isConnected()) {
        std::cout << "Warning: Attempting to send message while not connected!" << std::endl;
        // 如果启用了自动重连但当前未连接，先尝试立即重连
        if (autoReconnect_) {
            connect();
        }
        return 0;
    }
    
    uint32_t seq = connectionHandler_->sendMessage(client_.connection(), msg);
    if (seq > 0) {
        std::cout << "Sent message with sequence: " << seq << std::endl;
    }
    return seq;
}

void MyProtoClient::setMessageCallback(const MessageCallback& cb) {
    messageCallback_ = cb;
    connectionHandler_->setMessageCallback(cb); // 设置到连接处理器
}



void MyProtoClient::setReconnectInterval(int intervalMs) {
    reconnectIntervalMs_ = intervalMs;
}

void MyProtoClient::enableAutoReconnect(bool enable) {
    autoReconnect_ = enable;
}

void MyProtoClient::onTimeout() {
    connectionHandler_->checkTimeoutMessages();
}