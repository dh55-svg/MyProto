#include <iostream>
#include "MyProtoServer.h"
#include "muduo/net/EventLoop.h"
MyProtoServer::MyProtoServer(EventLoop* loop, const muduo::net::InetAddress& listenAddr, const std::string& nameArg)
    : server_(loop, listenAddr, nameArg),
      connectionHandler_(new ConnectionHandler()),
      businessHandler_(new BusinessHandler()) {
    
    // 设置连接处理器和业务处理器的相互引用
    connectionHandler_->setBusinessHandler(businessHandler_);
    businessHandler_->setConnectionHandler(connectionHandler_);
    
    // 设置TcpServer的回调
    server_.setConnectionCallback(
        std::bind(&ConnectionHandler::onConnection, connectionHandler_, std::placeholders::_1)
    );
    
    server_.setMessageCallback(
        std::bind(&ConnectionHandler::onMessage, connectionHandler_, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3)
    );
    
    server_.setWriteCompleteCallback(
        std::bind(&ConnectionHandler::onWriteComplete, connectionHandler_, std::placeholders::_1)
    );
    
    // 设置定时器，定期检查超时消息
    loop->runEvery(0.5, std::bind(&MyProtoServer::onTimeout, this));
}

MyProtoServer::~MyProtoServer() {
    stop();
}

void MyProtoServer::start() {
    std::cout << "Starting MyProtoServer on " << server_.ipPort() << std::endl;
    server_.start();
}

void MyProtoServer::stop() {
    server_.getLoop()->queueInLoop([this]() {
        server_.getLoop()->quit();
    });
}

std::shared_ptr<BusinessHandler> MyProtoServer::getBusinessHandler() {
    return businessHandler_;
}

void MyProtoServer::onTimeout() {
    connectionHandler_->checkTimeoutMessages();
}