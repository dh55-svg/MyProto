#include "ReliableMsgManager.h"
#include "muduo/net/TcpConnection.h"
#include "myproto.h"

ReliableMsgManager::ReliableMsgManager() : nextSequence_(1) {
}

ReliableMsgManager::~ReliableMsgManager() {
}

/**
 * 发送可靠消息的核心方法
 * @param conn TCP连接指针，用于发送消息
 * @param msg 要发送的原始消息
 * @return 返回分配的唯一序列号
 */
// 修改sendReliableMessage方法，添加更多调试输出
uint32_t ReliableMsgManager::sendReliableMessage(const muduo::net::TcpConnectionPtr& conn, const MyProtoMsg& msg) {
    if (!conn || !conn->connected()) {
        std::cout << "Error: Connection not valid or disconnected" << std::endl;
        return 0;
    }

    std::lock_guard<std::mutex> lock(mutex_);
    
    // 获取连接名称
    std::string connName = conn->name();
    
    // 分配唯一序列号
    uint32_t sequence = nextSequence_++;
    
    // 保存消息到待确认列表
    PendingMessage pendingMsg;
    pendingMsg.msg = msg;
    pendingMsg.msg.head.sequence = sequence; // 设置消息序列号
    // 关键修改：显式设置版本号为1（系统支持的版本）
    pendingMsg.msg.head.version = 1;
    pendingMsg.msg.head.type = 0; // 数据消息类型

    pendingMsg.sendTime = std::chrono::steady_clock::now();
    pendingMsg.retryCount = 0;
    
    pendingMessages_[connName][sequence] = pendingMsg;
    
    // 保存连接指针到映射中（使用weak_ptr避免循环引用）
    connectionMap_[connName] = std::weak_ptr<muduo::net::TcpConnection>(conn);
    
    // 编码并发送消息
    MyProtoEncode encoder;
    uint32_t len = 0;
    uint8_t* data = encoder.encode(const_cast<MyProtoMsg*>(&pendingMsg.msg), len);
    
    if (data && len > 0) {
        conn->send(data, len);
        std::cout << "Message encoded and sent successfully, length: " << len << " bytes" << std::endl;
        delete[] data;
    } else {
        std::cout << "Failed to encode message" << std::endl;
    }
    
    return sequence;
}

// 处理接收到的确认消息
void ReliableMsgManager::processAckMessage(const muduo::net::TcpConnectionPtr& conn, const MyProtoMsg& msg) {
    if (!conn || !conn->connected()) {
        return;
    }
    
    std::lock_guard<std::mutex> lock(mutex_);
    
    std::string connName = conn->name();
    uint32_t sequence = msg.head.sequence;
    
    // 查找并移除已确认的消息
    auto connIt = pendingMessages_.find(connName);
    if (connIt != pendingMessages_.end()) {
        auto msgIt = connIt->second.find(sequence);
        if (msgIt != connIt->second.end()) {
            // 消息已确认，从待确认列表中删除
            connIt->second.erase(msgIt);
            
            // 如果该连接没有待确认消息，清理连接映射
            if (connIt->second.empty()) {
                pendingMessages_.erase(connIt);
            }
        }
    }
}

// 处理接收到的数据消息（返回是否为新消息）
//就是将新来的数据消息进行去重处理，已经处理过的消息就不再处理，返回false
bool ReliableMsgManager::processDataMessage(const muduo::net::TcpConnectionPtr& conn, const MyProtoMsg& msg) {
    if (!conn || !conn->connected()) {
        return false;
    }
    
    std::lock_guard<std::mutex> lock(mutex_);
    
    std::string connName = conn->name();
    uint32_t sequence = msg.head.sequence;
    
    // 增加消息有效性检查
    // 1. 检查版本号是否支持
    if (msg.head.version != 0 && msg.head.version != 1) {
        std::cout << "[ReliableManager] Invalid message version: " << static_cast<int>(msg.head.version) << ", skipping" << std::endl;
        return false;
    }
    
    // 2. 检查消息类型是否合理（0-数据，1-确认，或其他自定义类型）
    // 注意：根据之前的修改，我们允许非标准消息类型，但要确保它不是明显的错误值
    if (msg.head.type == '{' || msg.head.type == '}' || msg.head.type == '[' || msg.head.type == ']') {
        // 这些字符很可能是JSON数据的一部分，而不是有效的消息类型
        std::cout << "[ReliableManager] Suspected JSON character as message type (0x" << std::hex << static_cast<int>(msg.head.type) << std::dec << "), skipping" << std::endl;
        return false;
    }
    
    // 3. 检查消息长度是否合理
    if (msg.head.len < MY_PROTO_HEAD_SIZE || msg.head.len > MY_PROTO_MAX_SIZE) {
        std::cout << "[ReliableManager] Invalid message length: " << msg.head.len << ", skipping" << std::endl;
        return false;
    }
    
    // 检查消息是否已处理过（去重）
    auto& processed = processedSequences_[connName];
    if (processed.count(sequence) > 0) {
        // 消息已处理过，发送确认但不进行业务处理
        sendAck(conn, sequence);
        std::cout << "[ReliableManager] Duplicate message detected, sequence: " << sequence << ", skipping" << std::endl;
        return false;
    }
    
    // 记录消息已处理
    processed.insert(sequence);
    
    // 发送确认消息
    sendAck(conn, sequence);
    
    // 消息为新消息，需要进行业务处理
    return true;
}

// 发送确认消息
void ReliableMsgManager::sendAck(const muduo::net::TcpConnectionPtr& conn, uint32_t sequence) {
    if (!conn || !conn->connected()) {
        return;
    }
    
    // 创建确认消息
    MyProtoMsg ackMsg;
    ackMsg.head.type = 1; // 设置为确认消息类型
    ackMsg.head.sequence = sequence;
    ackMsg.head.version = 1; // 设置版本号为1
    
    // 编码并发送确认消息
    MyProtoEncode encoder;
    uint32_t len = 0;
    uint8_t* data = encoder.encode(&ackMsg, len);
    
    if (data && len > 0) {
        conn->send(data, len);
        delete[] data;
    }
}

// 实现从连接名称获取连接指针的方法
muduo::net::TcpConnectionPtr ReliableMsgManager::getConnectionByName(const std::string& connName) {
    auto it = connectionMap_.find(connName);
    if (it != connectionMap_.end()) {
        // 尝试将弱引用升级为强引用
        muduo::net::TcpConnectionPtr conn = it->second.lock();
        if (conn && conn->connected()) {
            return conn;
        }
        // 连接已失效，清理映射
        connectionMap_.erase(it);
    }
    return nullptr;
}


/**
 * 检查并处理超时消息的核心方法
 * 该方法会遍历所有待确认的消息，检查是否超时，如果超时则进行重传或标记失败
 */
void ReliableMsgManager::checkTimeoutMessages() {
    // 加锁保证线程安全，防止并发访问导致的数据竞争
    std::lock_guard<std::mutex> lock(mutex_);
    // 获取当前时间，用于计算消息是否超时
    auto now = std::chrono::steady_clock::now();
    
    // 遍历所有连接的待确认消息列表
    for (auto& connPair : pendingMessages_) {
        // 获取连接名称和该连接对应的消息映射表
        std::string connName = connPair.first;
        auto& msgMap = connPair.second;
        
        // 遍历该连接下的所有待确认消息（使用迭代器以便在遍历时删除元素）
        for (auto it = msgMap.begin(); it != msgMap.end();) {
            // 获取当前消息和其发送时间
            auto& pendingMsg = it->second;
            // 计算消息已发送的时间（毫秒）
            auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(now - pendingMsg.sendTime).count();
            
            // 检查消息是否超时
            if (duration > RETRY_INTERVAL_MS) {
                // 检查是否超过最大重试次数
                if (pendingMsg.retryCount < MAX_RETRY_COUNT) {
                    // 增加重试计数并更新发送时间
                    pendingMsg.retryCount++;
                    pendingMsg.sendTime = now;
                    
                    try {
                        // 通过连接名称获取连接指针
                        muduo::net::TcpConnectionPtr conn = getConnectionByName(connName);
                        
                        // 检查连接是否有效且已连接
                        if (conn && conn->connected()) {
                            // 确保重发消息时版本号正确设置为1
                            pendingMsg.msg.head.version = 1;
                            // 创建编码器并编码消息
                            MyProtoEncode encoder;
                            uint32_t len = 0;
                            uint8_t* data = encoder.encode(const_cast<MyProtoMsg*>(&pendingMsg.msg), len);
                            
                            if (data && len > 0) {
                                // 发送重编码后的消息
                                conn->send(data, len);
                                // 输出调试信息
                                std::cout << "Retrying message, sequence: " << it->first << ", retry count: " << pendingMsg.retryCount << std::endl;
                                // 释放编码后的缓冲区内存
                                delete[] data;
                                // 移动到下一个消息
                                ++it;
                            } else {
                                // 编码失败，从待确认列表中删除该消息
                                std::cout << "Failed to encode message during retry, sequence: " << it->first << std::endl;
                                it = msgMap.erase(it);
                            }
                        } else {
                            // 连接无效或已断开，从待确认列表中删除该消息
                            std::cout << "Connection invalid during retry, sequence: " << it->first << std::endl;
                            it = msgMap.erase(it);
                        }
                    } catch (const std::exception& e) {
                        // 捕获并处理重传过程中的异常
                        std::cerr << "Error during message retry: " << e.what() << std::endl;
                        // 异常情况下也从待确认列表中删除该消息
                        it = msgMap.erase(it);
                    }
                } else {
                    // 超过最大重试次数，标记消息发送失败
                    std::cout << "Message failed after max retries, sequence: " << it->first << std::endl;
                    // 从待确认列表中删除该消息
                    it = msgMap.erase(it);
                }
            } else {
                // 消息未超时，继续检查下一个消息
                ++it;
            }
        }
        
        // 如果该连接下没有待确认消息了，清理对应的连接条目
        if (msgMap.empty()) {
            pendingMessages_.erase(connPair.first);
        }
    }
}

// 修改cleanupConnection方法，确保清理所有相关资源
void ReliableMsgManager::cleanupConnection(const std::string& connName) {
    std::lock_guard<std::mutex> lock(mutex_);
    
    // 清理该连接的所有待处理消息和已处理序列号
    pendingMessages_.erase(connName);
    processedSequences_.erase(connName);
    
    // 清理连接映射
    connectionMap_.erase(connName);
}