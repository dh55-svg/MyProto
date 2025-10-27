#include "ConnectionHandler.h"
#include "BusinessHandler.h"
#include <iostream>

// 修复构造函数，确保正确初始化connectionCallback_
ConnectionHandler::ConnectionHandler() {
    decoder_.init();
    connectionCallback_ = nullptr; // 确保回调初始化为nullptr
    std::cout << "[Handler] Constructor: connectionCallback_ initialized to nullptr" << std::endl;
}

ConnectionHandler::~ConnectionHandler() {
}

void ConnectionHandler::setBusinessHandler(std::shared_ptr<BusinessHandler> handler) {
    businessHandler_ = handler;
}

void ConnectionHandler::setMessageCallback(const MessageCallback& cb) {
    messageCallback_ = cb;
}

// 修改onMessage方法，在业务处理后触发回调
void ConnectionHandler::onMessage(const TcpConnectionPtr& conn, muduo::net::Buffer* buf, muduo::Timestamp time) {
    std::cout << time.toString() << " OnMessage called for connection: " << conn->name() 
              << ", readable bytes: " << buf->readableBytes() << std::endl;
    
    // 从buffer中读取数据并解析
    while (buf->readableBytes() > 0) {
        size_t readable = buf->readableBytes();
        std::cout << "[Handler] Attempting to parse " << readable << " bytes" << std::endl;
        
        if (decoder_.parser(const_cast<void*>(static_cast<const void*>(buf->peek())), readable)) {
            std::cout << "[Handler] Parser succeeded, retrieving " << readable << " bytes" << std::endl;
            buf->retrieve(readable);
            
            // 处理解析出的消息
            std::cout << "[Handler] Messages in queue: " << (decoder_.empty() ? "0" : "not empty") << std::endl;
            while (!decoder_.empty()) {
                auto msg = decoder_.front();
                decoder_.pop();
                std::cout << "[Handler] Processing message, type: " << msg->head.type 
                          << ", serverId: " << msg->head.server << std::endl;
                
                // 根据消息类型处理
                if (msg->head.type == 1) { // 确认消息
                    reliableManager_.processAckMessage(conn, *msg);
                } else { // 数据消息
                    if (reliableManager_.processDataMessage(conn, *msg)) {
                        std::cout << "[Handler] Processing new data message, sending to business handler" << std::endl;
                        // 新消息，交给业务层处理
                        if (businessHandler_) {
                            std::cout << "[Handler] Business handler exists, calling handleMessage" << std::endl;
                            businessHandler_->handleMessage(conn, msg);
                        } else {
                            std::cout << "[Handler] WARNING: No business handler set!" << std::endl;
                        }
                        // 触发用户回调
                        if (messageCallback_) {
                            messageCallback_(conn, msg);
                        }
                    } else {
                        std::cout << "[Handler] Duplicate or invalid message, skipped" << std::endl;
                    }
                }
            }
        } else {
            // 数据不完整，等待更多数据
            std::cout << "[Handler] Parser failed, waiting for more data" << std::endl;
            break;
        }
    }
}

void ConnectionHandler::onWriteComplete(const TcpConnectionPtr& conn) {
    // 可用于流量控制或统计
    std::cout << "Write complete for connection: " << conn->name() << std::endl;
}

uint32_t ConnectionHandler::sendMessage(const TcpConnectionPtr& conn, const MyProtoMsg& msg) {
    return reliableManager_.sendReliableMessage(conn, msg);
}

void ConnectionHandler::checkTimeoutMessages() {
    reliableManager_.checkTimeoutMessages();
}

// 在文件中添加onConnection方法实现
// 确保onConnection方法中的回调触发部分正确
void ConnectionHandler::onConnection(const TcpConnectionPtr& conn) {
    if (conn->connected()) {
        std::cout << "[Handler] New connection established: " << conn->name() << " from "
                  << conn->peerAddress().toIpPort() << " to "
                  << conn->localAddress().toIpPort() << std::endl;
        // 添加连接计数和状态日志
        std::cout << "[Handler] Current connection status: CONNECTED" << std::endl;
    } else {
        std::cout << "[Handler] Connection closed: " << conn->name() << std::endl;
        // 连接关闭时清理相关资源
        reliableManager_.cleanupConnection(conn->name());
        // 通知连接断开事件给监听者
        if (connectionCallback_) {
            std::cout << "[Handler] Triggering connection callback" << std::endl;
            connectionCallback_(conn);
        }
    }
}

// 在setMessageCallback方法之后添加setConnectionCallback方法的实现
// 修改setConnectionCallback方法，添加调试日志
// 保留setConnectionCallback方法的实现，但去掉重复的构造函数和游离的代码块
void ConnectionHandler::setConnectionCallback(const ConnectionCallback& cb) {
    std::cout << "[Handler] Setting connection callback" << std::endl;
    connectionCallback_ = cb;
}