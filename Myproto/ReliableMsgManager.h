#ifndef __RELIABLE_MSG_MANAGER_H
#define __RELIABLE_MSG_MANAGER_H

#include <mutex>
#include <unordered_map>
#include <queue>
#include <chrono>
#include <unordered_set>
#include "myproto.h"
#include "muduo/net/TcpConnection.h"

// 消息重传配置
const int MAX_RETRY_COUNT = 3; // 最大重传次数
const int RETRY_INTERVAL_MS = 1000; // 重传间隔（毫秒）

// 等待确认的消息信息
struct PendingMessage {
    MyProtoMsg msg; // 消息内容
    std::chrono::steady_clock::time_point sendTime; // 发送时间
    int retryCount; // 已重传次数
};

// 可靠消息管理器
class ReliableMsgManager {
public:
    ReliableMsgManager();
    ~ReliableMsgManager();
    
    // 发送可靠消息
    uint32_t sendReliableMessage(const muduo::net::TcpConnectionPtr& conn, const MyProtoMsg& msg);
    
    // 处理接收到的确认消息
    void processAckMessage(const muduo::net::TcpConnectionPtr& conn, const MyProtoMsg& msg);
    
    // 处理接收到的数据消息（返回是否为新消息）
    bool processDataMessage(const muduo::net::TcpConnectionPtr& conn, const MyProtoMsg& msg);
    
    // 检查并处理超时消息
    void checkTimeoutMessages();
    // 清理连接相关资源
    void cleanupConnection(const std::string& connName);
    
private:
    std::mutex mutex_; // 保护共享数据
    uint32_t nextSequence_; // 下一个要使用的序列号
    
    // 按连接保存待确认的消息
    std::unordered_map<std::string, std::unordered_map<uint32_t, PendingMessage>> pendingMessages_;
    
    // 按连接保存已处理的消息序列号，用于去重
    std::unordered_map<std::string, std::unordered_set<uint32_t>> processedSequences_;
    
    // 修改：保存连接名称到连接弱指针的映射，避免循环引用
    std::unordered_map<std::string, std::weak_ptr<muduo::net::TcpConnection>> connectionMap_;
    
    // 发送确认消息
    void sendAck(const muduo::net::TcpConnectionPtr& conn, uint32_t sequence);
    
    // 新增：从连接名称获取连接指针
    muduo::net::TcpConnectionPtr getConnectionByName(const std::string& connName);
    
    
};

#endif // __RELIABLE_MSG_MANAGER_H