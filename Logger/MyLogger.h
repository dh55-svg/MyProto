#pragma once
#include <string>
using namespace std;
enum class LogLevel{
    Debug,
    Info,
    Warn,
    Error,
    FATAL
};
class MyLogger{
public:
  static void setLogLevel(LogLevel level);
  static void Debug(const string& module,const string& message);
  static void Info(const string& module,const string& message);
  static void Warn(const string& module,const string& message);
  static void Error(const string& module,const string& message);
  static void FATAL(const string& module,const string& message);
private:
  static LogLevel currentLevel_;
  static void Log(LogLevel level,const string& module,const string& message);
  static string getTimestamp();
  static string levelToString(LogLevel level);
};