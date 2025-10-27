#include <iostream>
#include <signal.h>
#include "MyProtoServer.h"
#include "muduo/net/EventLoop.h"  // 添加EventLoop的头文件
using namespace std;
using namespace muduo;
using namespace muduo::net;


EventLoop* g_loop = nullptr;

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