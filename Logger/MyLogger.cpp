#include "MyLogger.h"
#include <iostream>
#include <chrono>
#include <iomanip>
LogLevel MyLogger::currentLevel_ = LogLevel::Debug;
void MyLogger::setLogLevel(LogLevel level)
{
  currentLevel_ = level;

}
void MyLogger::Debug(const string& module,const string& message)
{
  Log(LogLevel::Debug,module,message);
}
void MyLogger::Info(const string& module,const string& message)
{
  Log(LogLevel::Info,module,message);
}
void MyLogger::Warn(const string& module,const string& message)
{
  Log(LogLevel::Warn,module,message);
}
void MyLogger::Error(const string& module,const string& message)
{
  Log(LogLevel::Error,module,message);
}
void MyLogger::FATAL(const string& module,const string& message)
{
  Log(LogLevel::FATAL,module,message);
}
void MyLogger::Log(LogLevel level,const string& module,const string& message)
{
  if(level<currentLevel_)
  {
    return;// 如果当前日志级别低于设置的级别，则不输出
  }
  string timestamp = getTimestamp();
  string levelStr = levelToString(level);
  if(level>=LogLevel::Error)
  {
    std::cerr << timestamp << " [" << levelStr << "] [" << module << "] " << message << std::endl;
  }else {
        std::cout << timestamp << " [" << levelStr << "] [" << module << "] " << message << std::endl;
  }
}
string MyLogger::getTimestamp()
{
  auto now=chrono::system_clock::now();
  auto now_c=chrono::system_clock::to_time_t(now);
  auto ms=chrono::duration_cast<chrono::milliseconds>(now.time_since_epoch())%1000;
  stringstream ss;
  ss<<put_time(localtime(&now_c),"%Y-%m-%d %H:%M:%S")
    <<setw(3)<<setfill('0')<<ms.count();
  return ss.str();
}
string MyLogger::levelToString(LogLevel level)
{
  switch(level)
  {
    case LogLevel::Debug: return "Debug";
    case LogLevel::Info: return "Info";
    case LogLevel::Warn: return "Warn";
    case LogLevel::Error: return "Error";
    case LogLevel::FATAL: return "FATAL";
    default: return "Unknown";
  }
}
