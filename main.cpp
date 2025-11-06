#include <iostream>
#include <signal.h>
#include "MyProtoServer.h"
#include "muduo/net/EventLoop.h"  // 添加EventLoop的头文件
#include <fstream>
#include <iomanip>
#include <chrono>
#include <sstream>
#include <mutex>
using namespace std;
using namespace muduo;
using namespace muduo::net;


EventLoop* g_loop = nullptr;

// 用于文件操作的互斥锁，确保线程安全
std::mutex fileMutex;
void signalHandler(int sig) {
    std::cout << "Received signal: " << sig << std::endl;
    if (g_loop) {
        g_loop->queueInLoop(std::bind(&EventLoop::quit, g_loop));
    }
}

// 业务处理示例
void handleEchoRequest(const TcpConnectionPtr& conn, const std::shared_ptr<MyProtoMsg>& msg, ConnectionHandler* connHandler) {
    std::cout << "[EchoHandler] handleEchoRequest called, connection: " << conn->name() << std::endl;
    std::cout << "[EchoHandler] Message body: " << msg->body.dump() << std::endl;
    try
    {
        auto now=chrono::system_clock::now();
        auto now_c=chrono::system_clock::to_time_t(now);
        stringstream ss;
        // 创建保存目录（如果不存在）
        system("mkdir -p received_echo_data");
        ss << "received_echo_data/" 
           << conn->peerAddress().toIpPort() << "_" 
           << std::put_time(std::localtime(&now_c), "%Y%m%d_%H%M%S") << "_" 
           << std::chrono::duration_cast<std::chrono::milliseconds>(now.time_since_epoch()).count() % 1000 
           << ".json";
        std::string filename = ss.str();
         // 加锁确保线程安全
        std::lock_guard<std::mutex> lock(fileMutex);
        // 打开文件并保存消息
        std::ofstream file(filename);
        if (file.is_open()) {
            // 保存消息的元数据
            json metadata;
           metadata["client_ip"] = conn->peerAddress().toIp();
            metadata["client_port"] = conn->peerAddress().port();
            metadata["connection_name"] = conn->name();
            // 创建临时变量保存packed结构体字段值
            uint16_t serverId = msg->head.server;
            uint8_t msgType = msg->head.type;
            uint32_t seq = msg->head.sequence;
            uint8_t ver = msg->head.version;
            // 使用临时变量赋值给JSON对象
            metadata["server_id"] = serverId;
            metadata["message_type"] = msgType;
            metadata["sequence"] = seq;
            metadata["version"] = ver;

            // 组合元数据和消息体
            json fileContent;
            fileContent["metadata"] = metadata;
            fileContent["data"] = msg->body;

            // 保存到文件
            file << std::setw(4) << fileContent << std::endl;
            file.close();
            std::cout << "[EchoHandler] Message saved to " << filename << std::endl;
        }
    }
    catch(const std::exception& e)
    {
        std::cerr << "[EchoHandler] Error saving message to file: " << e.what() << std::endl;
    }
    
    
    json responseBody;
    responseBody["echo"] = msg->body;
    responseBody["status"] = "success";
    
    MyProtoMsg responseMsg;
    responseMsg.head.version = 1;
    // 移除对已删除的magic字段的设置
    responseMsg.head.server = msg->head.server; // 与请求相同的服务ID
    responseMsg.head.sequence = 0;
    responseMsg.head.type = 0;
    responseMsg.body = responseBody;
    
    std::cout << "[EchoHandler] Sending response: " << responseMsg.body.dump() << std::endl;
    connHandler->sendMessage(conn, responseMsg);
}

int main(int argc, char* argv[]) {
    // 解析命令行参数
    int port = 8888;
    if (argc > 1) {
        port = atoi(argv[1]);

    }
    
    // 设置信号处理
    signal(SIGINT, signalHandler);
    signal(SIGTERM, signalHandler);
    
    // 创建事件循环和服务器
    EventLoop loop;
    g_loop = &loop;
    
    InetAddress listenAddr(port);
    MyProtoServer server(&loop, listenAddr, "MyProtoServer");
    
    // 注册业务处理函数
    auto businessHandler = server.getBusinessHandler();
    businessHandler->registerHandler(1, handleEchoRequest); // 注册回显服务
    
    // 启动服务器
    server.start();
    
    // 运行事件循环
    loop.loop();
    
    return 0;
}