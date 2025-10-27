#ifndef __MY_PROTO_H
#define __MY_PROTO_H

#include <stdint.h>
#include <stdio.h>
#include <queue>
#include <vector>
#include <iostream>
#include <cstring>
#include "json.hpp"

using namespace std;
using json = nlohmann::json;

const uint32_t MY_PROTO_MAX_SIZE = 10*1024*1024; //10M协议中数据最大
const uint32_t MY_PROTO_HEAD_SIZE = 14; // 协议头大小由15改为14（移除了1字节的magic字段）

// 添加CRC相关常量定义
extern const uint16_t CRC_INITIAL_VALUE; // CRC初始值
extern const uint16_t CRC_POLYNOMIAL;    // CRC多项式

// 协议头字段偏移量常量
extern const uint32_t VERSION_OFFSET;   // 版本字段偏移量
extern const uint32_t SERVER_OFFSET;    // 服务号字段偏移量
extern const uint32_t LEN_OFFSET;       // 长度字段偏移量
extern const uint32_t CRC_OFFSET;       // CRC字段偏移量
extern const uint32_t SEQUENCE_OFFSET;  // 序列号字段偏移量
extern const uint32_t TYPE_OFFSET;      // 消息类型字段偏移量

typedef enum MyProtoParserStatus //协议解析的状态
{
	ON_PARSER_INIT = 0, //初始状态
	ON_PARSER_HEAD = 1, //解析头部
	ON_PARSER_BODY = 2, //解析数据
}MyProtoParserStatus;

//协议头部 - 添加packed属性强制紧凑布局
struct MyProtoHead {
    uint8_t version; //协议版本号
    uint16_t server; //协议复用的服务号
    uint32_t len; //协议长度
    uint16_t crc; //CRC校验值
    uint32_t sequence; //协议序列号
    uint8_t type; //协议类型 0-数据 1确认消息
} __attribute__((packed)); // 重要：强制结构体紧凑布局

//协议消息体
struct MyProtoMsg
{
	MyProtoHead head; //协议头
	json body; //协议体
};

// 增加CRC计算函数声明
uint16_t calculateCRC(const uint8_t* data, size_t length);
bool validateJsonContent(const json& j);
//公共函数
//打印协议数据信息
void printMyProtoMsg(MyProtoMsg& msg);

//协议封装类
class MyProtoEncode
{
public:
	//协议消息体封装函数：传入的pMsg里面只有部分数据，比如Json协议体，服务号，我们对消息编码后会修改长度信息，这时需要重新编码协议
	uint8_t* encode(MyProtoMsg* pMsg, uint32_t& len); //返回长度信息，用于后面socket发送数据
private:
	//协议头封装函数
	void headEncode(uint8_t* pData,MyProtoMsg* pMsg);
};



//协议解析类
class MyProtoDecode
{
private:
	MyProtoMsg mCurMsg; //当前解析中的协议消息体
	queue<std::shared_ptr<MyProtoMsg>> mMsgQ; //解析好的协议消息队列
	vector<uint8_t> mCurReserved; //未解析的网络字节流，可以缓存所有没有解析的数据（按字节）
	MyProtoParserStatus mCurParserStatus; //当前接受方解析状态
public:
	void init(); //初始化协议解析状态
	void clear(); //清空解析好的消息队列
	bool empty(); //判断解析好的消息队列是否为空
	void pop();  //出队一个消息

	std::shared_ptr<MyProtoMsg> front(); //获取一个解析好的消息
	bool parser(void* data,size_t len); //从网络字节流中解析出来协议消息，len是网络中的字节流长度，通过socket可以获取
private:
	bool parserHead(uint8_t** curData,uint32_t& curLen,
		uint32_t& parserLen,bool& parserBreak); //用于解析消息头
	bool parserBody(uint8_t** curData,uint32_t& curLen,
		uint32_t& parserLen,bool& parserBreak); //用于解析消息体
};

#endif