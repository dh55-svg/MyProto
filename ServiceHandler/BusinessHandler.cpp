#include "BusinessHandler.h"
#include "ConnectionHandler.h"
#include <iostream>

BusinessHandler::BusinessHandler() {
}

BusinessHandler::~BusinessHandler() {
}

void BusinessHandler::setConnectionHandler(std::shared_ptr<ConnectionHandler> handler) {
    connectionHandler_ = handler;
}

/**
 * @brief 注册业务处理函数
 * 
 * 此函数用于将特定服务器ID的业务处理函数注册到消息处理映射表中。当系统收到指定服务器ID的消息时，
 * 将自动调用对应的处理函数进行业务逻辑处理。
 * 
 * @param serverId 服务器ID，用于标识不同的业务处理模块
 * @param handler 消息处理函数，符合MessageHandler类型定义的回调函数
 *                函数签名: void(const TcpConnectionPtr&, const std::shared_ptr<MyProtoMsg>&, ConnectionHandler*)
 */
void BusinessHandler::registerHandler(uint16_t serverId, const MessageHandler& handler) {
    messageHandlers_[serverId] = handler;
}

void BusinessHandler::handleMessage(const TcpConnectionPtr& conn, const std::shared_ptr<MyProtoMsg>& msg) {
    auto it = messageHandlers_.find(msg->head.server);
    if (it != messageHandlers_.end()) {
        try {
            // 调用对应的业务处理函数
            it->second(conn, msg, connectionHandler_.get());
        } catch (const std::exception& e) {
            std::cerr << "Business handler exception: " << e.what() << std::endl;
            
            // 发送错误响应
            json errorResponse;
            errorResponse["error"] = e.what();
            errorResponse["code"] = -1;
            sendResponse(conn, msg->head.server, errorResponse);
        }
    } else {
        std::cerr << "No handler registered for serverId: " << msg->head.server << std::endl;
    }
}

/**
 * @brief 发送响应消息给客户端
 * 
 * 此函数用于构造响应消息并通过连接处理器发送给指定的TCP连接。它会创建一个标准格式的
 * MyProtoMsg消息，设置必要的头部信息，并将业务响应数据作为消息体。
 * 
 * @param conn TCP连接指针，指向需要发送响应的客户端连接
 * @param serverId 服务器ID，标识响应来自哪个服务模块
 * @param responseBody 响应体数据，使用json格式存储的业务响应内容
 * @return uint32_t 发送结果，成功返回非零值（通常是发送的字节数），失败返回0
 */
uint32_t BusinessHandler::sendResponse(const TcpConnectionPtr& conn, uint16_t serverId, const json& responseBody) {
    // 检查连接处理器是否已设置
    if (!connectionHandler_) {
        return 0;
    }
    
    // 创建响应消息对象
    MyProtoMsg responseMsg;
    // 设置消息版本号
    responseMsg.head.version = 1;
    // 移除魔数设置
    // 设置服务器ID
    responseMsg.head.server = serverId;
    // 序列号设为0，将由ReliableMsgManager在发送时分配
    responseMsg.head.sequence = 0; // 将由ReliableMsgManager分配
    // 设置消息类型为数据消息
    responseMsg.head.type = 0; // 数据消息
    // 设置响应体
    responseMsg.body = responseBody;
    
    // 通过连接处理器发送消息并返回发送结果
    return connectionHandler_->sendMessage(conn, responseMsg);
}