#include <iostream>
#include <thread>
#include <chrono>
#include "MyProtoClient.h"
#include "muduo/net/EventLoop.h" 
using namespace std;
using namespace muduo;
using namespace muduo::net;


void sendTestMessage(MyProtoClient& client) {
    if (!client.isConnected()) {
        cout << "Not connected to server, waiting..." << endl;
        this_thread::sleep_for(chrono::seconds(1));
        return;
    }
    
    // 创建测试消息
    MyProtoMsg msg;
    msg.head.version = 1;
    // 移除魔数设置
    msg.head.server = 1; // 回显服务
    msg.head.type=0;
    
    // 构建JSON消息体
    json body;
    body["test"] = "Hello, muduo!";
    body["timestamp"] = time(nullptr);
    body["sequence"] = rand();
    
    msg.body = body;
    
    cout << "sendTestMessage called, msg body: " << body.dump() << endl;
    
    // 发送消息
    uint32_t seq = client.sendMessage(msg);
    
    cout << "Sent message with sequence: " << seq << endl;
}


int main(/*int argc, char* argv[]*/) {
    // 解析命令行参数
    int port = 8888;
    string ip = "127.0.0.1";
    
    // 创建事件循环和客户端
    EventLoop loop;
    InetAddress serverAddr(ip, port);
    MyProtoClient client(&loop, serverAddr, "MyProtoClient");
    
    // 连接服务器
    client.connect();
    
    // 启动一个线程定期发送测试消息
    thread sender([&client]() {
        this_thread::sleep_for(chrono::seconds(2)); // 等待连接建立
        
        for (int i = 0; i < 5; ++i) {
            sendTestMessage(client);
            this_thread::sleep_for(chrono::seconds(1));
        }
        
        // 发送完消息后等待一会再退出
        this_thread::sleep_for(chrono::seconds(2));
        client.disconnect();
    });
    
    // 运行事件循环
    loop.loop();
    
    if (sender.joinable()) {
        sender.join();
    }
    
    return 0;
}