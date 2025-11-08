#include <iostream>
#include <string>
#include <memory>
using namespace std;
// 抽象主题：原对象和代理的统一接口
class Subject{
public:
    virtual void request() const=0;
    virtual ~Subject()=default;
};
// 真实主题：重量级对象（如大文件加载）
class RealSubject : public Subject {
private:
  string data;
public:
  RealSubject(){
    // 模拟耗时初始化（如加载大文件）
        cout << "真实对象：加载大文件（耗时操作）" << endl;
        data = "大文件数据...";
  }
  void request() const override {
    cout << "真实对象：处理请求，数据内容：" << data << endl;
  }
};
// 代理主题：控制对 RealSubject 的访问（延迟加载）
class Proxy : public Subject {
private:
  mutable unique_ptr<RealSubject> realSubject_;// 懒加载真实对象
public:
  void request() const override {
    if (!realSubject_) {
      realSubject_ = make_unique<RealSubject>();
    }
    realSubject_->request();
  }
};
int main()
{
    cout << "客户端：创建代理对象" << endl;
    Proxy proxy;
    cout << "客户端：通过代理处理请求" << endl;
    proxy.request(); // 真实对象在此时才被创建
    return 0;
}