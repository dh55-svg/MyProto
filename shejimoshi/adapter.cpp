#include <iostream>
#include <memory>
using namespace std;
//类适配器
class Target{
public:
  virtual void request() const=0;
  virtual ~Target()=default;
};
// 适配者（第三方库/旧系统的接口，与 Target 不兼容）
class Adaptee{
public:
  void specificRequest() const {
    cout <<  "第三方库：执行具体功能" << endl;
  }
};
// 适配器（多继承：继承 Target 接口，继承 Adaptee 实现）
class Adapter:public Target,private Adaptee{
public:
  void request() const override {
    // 调用适配者的具体请求方法
    specificRequest();// 调用适配者的接口，适配成 Target 接口
  }
};
void clientCode(const Target& target) {
    target.request();
}

//对象适配
class Adapter2 : public Target {
public:
  Adapter2(unique_ptr<Adaptee> adaptee) : adaptee_(std::move(adaptee)) {}
  void request() const override {
    adaptee_->specificRequest();
  }
private:
  std::unique_ptr<Adaptee> adaptee_;
};
int main()
{
  //类适配
    Adapter adapter;
    clientCode(adapter);
    //对象适配
    auto adaptee=make_unique<Adaptee>();
    Adapter2 adapter2(std::move(adaptee));
    clientCode(adapter2);
    return 0;
}