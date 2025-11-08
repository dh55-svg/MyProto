#include <iostream>
#include <string>
#include <memory>
using namespace std;
class Component{
public:
  virtual void operation() const=0;
  virtual ~Component()=default;
};
// 具体组件：核心功能实现（如业务逻辑）
class ConcreteComponent : public Component {
public:
  void operation() const override {
    cout << "具体组件的操作" << endl;
  }
};
//装饰器抽象类（继承 Component，持有 Component 对象）
class Decorator : public Component {
public:
  Decorator(unique_ptr<Component> component) : component_(move(component)) {}
  virtual void operation() const override=0;
protected:
  unique_ptr<Component> component_;
};
// 具体装饰器：扩展功能（如日志、权限校验等）
class LogDecorator : public Decorator {
public:
  using Decorator::Decorator;
  void operation() const override {
    cout << "日志装饰器：记录操作前" << endl;
    component_->operation();
    cout << "日志装饰器：记录操作后" << endl;
  }
};
int main()
{
  auto core=make_unique<ConcreteComponent>();
  cout << "=== 仅核心功能 ===" << endl;
  core->operation();
  // 2. 核心功能 + 日志
  auto coreWithLog = make_unique<LogDecorator>(make_unique<ConcreteComponent>());
  cout << "\n=== 核心功能 + 日志 ===" << endl;
  coreWithLog->operation();
}