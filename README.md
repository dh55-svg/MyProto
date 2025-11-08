
# 设计模式

## 1、单例模式

优点：全局唯一、节省资源；缺点：耦合高、难以测试、不适合多线程写操作

**避免使用场景：**

​	当实例可能需要多个变体时：例如，一个日志系统可能需要同时输出到文件和控制台，就需要多个实例。

​	 当全局状态会导致问题时：

​	当需要明确的生命周期管理时：单例的生命周期通常跨越整个程序运行期，无法在不需要时释放资源

​	在高度并行的环境中：虽然可以实现线程安全的单例，但在高并发场景下可能成为性能瓶颈，特别是当单例对象内部有复杂的状态管理或频繁访问时。

​	当依赖注入更合适时

​	当违反单一职责原则时：如果一个类同时负责管理自己的实例唯一性和实现业务逻辑，就违反了单一职责原则，应当将这两部分职责分离。

​	当需要序列化和反序列化时：单例模式与对象序列化通常不兼容，因为反序列化可能会创建新的实例，破坏单例的唯一性保证。

## 2、工厂设计模式

定义一个 **创建对象的接口**，让子类决定实例化哪个类（解耦 “对象创建” 与 “业务逻辑”），体现开放封闭原则。

适用场景

- 产品（对象）类型不确定，后续可能新增产品（如支付方式、日志输出方式）；
- 希望隐藏产品创建细节（如复杂的初始化流程）。

符合开闭和依赖倒置

```c++
#include <iostream>
#include <memory>
#include <string>
using namespace std;
class Payment{
public:
  virtual void pay(double amount) const=0;
  virtual ~Payment()=default;
};
class Alipay:public Payment{
public:
  void pay(double amount) const override {
    cout << "支付宝支付 " << amount << " 元" << endl;
  }
};
class WechatPay : public Payment {
public:
    void pay(double amount) const override {
        cout << "微信支付 " << amount << " 元" << endl;
    }
};
//抽象工厂类
class PaymentFactory {
public:
  virtual unique_ptr<Payment> createPayment() const = 0;
  virtual ~PaymentFactory()=default;
};
//具体工厂
class AlipayFactory : public PaymentFactory {
  public:
    unique_ptr<Payment> createPayment() const override{
      return make_unique<Alipay>();
    }
};
class WechatPayFactory : public PaymentFactory {
public:
    unique_ptr<Payment> createPayment() const override {
        return make_unique<WechatPay>();
    }
};
// 业务逻辑（依赖抽象工厂，不依赖具体产品）
void processPayment(const PaymentFactory& factory, double amount) {
    auto payment = factory.createPayment();
    payment->pay(amount);
}
int main()
{
    AlipayFactory alipayFactory;
    WechatPayFactory wechatPayFactory;

    processPayment(alipayFactory, 100.0); // 使用支付宝支付
    processPayment(wechatPayFactory, 200.0); // 使用微信支付

    return 0;
}
```

## 3、适配器

将一个类的接口 **转换成客户端期望的另一个接口**，让原本接口不兼容的类能一起工作（“接口转换器”）。

#### 适用场景

- 集成第三方库（接口不匹配现有系统）；
- 重构旧系统（旧接口需适配新接口，避免大规模修改代码）。

两种方式：类适配和方法适配

```c++
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
```

## 4、装饰器模式

动态地给对象 **添加额外功能**，且不改变其原有的接口（比继承更灵活的功能扩展方式）。

#### 适用场景

- 需给对象动态添加 / 移除功能（如日志增强、缓存增强、权限校验）；
- 避免用继承产生过多子类（“类爆炸” 问题）。

```c++
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
```

## 5、代理模式

为其他对象提供一种 **代理（占位符）**，控制对原对象的访问（如权限控制、延迟加载、日志记录）。

#### 适用场景

- 延迟加载（重量级对象，如大文件、数据库连接，用到时才创建）；

- 权限控制（仅允许特定用户访问原对象）；

- 远程代理（访问远程服务器对象，代理处理网络通信）。

  ```c++
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
  ```

  

## 6、观察者模式

定义 **对象间的一对多依赖关系**，当一个对象（主题）状态变化时，所有依赖它的对象（观察者）会自动收到通知并更新（如订阅 - 发布模式）

#### 适用场景

- 事件驱动场景（如按钮点击、消息通知、状态同步）；

- 一个对象状态变化需联动多个对象更新（如订单支付成功后，通知库存、物流、积分系统）。

  ```c++
  #include <iostream>
  #include <string>
  #include <vector>
  #include <memory>
  #include <algorithm>
  #include <iostream>
  using namespace std;
  // 前向声明：观察者需要知道主题接口
  class Subject;
  
  // 观察者抽象类 - 移到 Subject 类前面定义
  class Observer {
  public:
      virtual void update(const string& msg) = 0;
      virtual ~Observer() = default;
  };
  
  // 主题（被观察者）抽象类
  class Subject {
  private:
      vector<weak_ptr<Observer>> observers_; // 使用弱指针避免循环引用
  
  public:
      // 注册观察者
      void attach(const shared_ptr<Observer>& observer) {
          observers_.push_back(observer);
      }
      
      void detach(const shared_ptr<Observer>& observer) {
          observers_.erase(
              remove_if(observers_.begin(), observers_.end(), [&](const weak_ptr<Observer>& wp) {
                  return wp.lock() == observer;
              }),
              observers_.end()
          );
      }
      
      // 通知所有观察者（模板方法）
      void notify(const string& msg) {
          for (auto it = observers_.begin(); it != observers_.end(); ) {
              if (auto obs = it->lock()) {
                  obs->update(msg);
                  ++it;
              } else {
                  // 观察者已被销毁，移除弱指针
                  it = observers_.erase(it);
              }
          }
      }
      
      virtual ~Subject() = default;
  };
  
  // 具体主题：订单
  class Order : public Subject {
  private:
      string order_id_;
      string status_;
  public:
      Order(const string& id) : order_id_(id), status_("未支付") {}
      
      void pay() {
          status_ = "已支付";
          notify("订单 " + order_id_ + " 状态更新为: " + status_);
      }
  };
  // 具体观察者1：库存系统
  class InventoryObserver : public Observer {
  public:
      void update(const string& msg) override {
          cout << "库存系统：" << msg << "，开始扣减库存" << endl;
      }
  };
  int main()
  {
    auto order=make_shared<Order>("ORDER12345");
    auto inventoryObserver=make_shared<InventoryObserver>();
    order->attach(inventoryObserver);
    order->pay();
    return 0;
  }
  ```

  ## 7、状态模式

  允许对象在 **内部状态变化时改变其行为**，使对象看起来像修改了它的类（如订单状态流转：未支付→已支付→已发货→已完成）。

#### 适用场景

- 对象状态多且状态间流转复杂（如订单、工作流、状态机）；
- 避免用 `if-else`/`switch` 判断状态（如 `if (status == UNPAID) { ... } else if (status == PAID) { ... }`）。

```c++
#include <iostream>
#include <string>
#include <memory>
using namespace std;
class Order;
// 抽象状态：订单状态接口
class OrderState{
public:
  virtual string getName() const=0;
  virtual void pay(Order& order)=0;
  virtual void ship(Order& order) = 0;   // 发货操作
  virtual void complete(Order& order) = 0; // 完成操作
  virtual ~OrderState() = default;
};
class UnpaidState : public OrderState {
public:
  string getName() const override {
    return "未支付";
  }
  void pay(Order& order) override; // 声明，后续实现
  void ship(Order& order) override {
    cout << "订单未支付，无法发货" << endl;
  }
  void complete(Order& order) override {
    cout << "订单未支付，无法完成" << endl;
  }
};
// 具体状态2：已支付
class PaidState : public OrderState {
public:
  string getName() const override {
    return "已支付";
  }
  void pay(Order& order) override {
    cout << "订单已支付，无需重复支付" << endl;
  }
  void ship(Order& order) override; // 声明，后续实现
  void complete(Order& order) override{
    cout << "订单已支付，无法完成" << endl;
  }
};
class ShippedState : public OrderState {
public:
  string getName() const override {
    return "已发货";
  }
  void pay(Order& order) override {
    cout << "订单已发货，无法支付" << endl;
  }
  void ship(Order& order) override {
    cout << "订单已发货，无法重复发货" << endl;
  }
  void complete(Order& order) override ;
};
// 具体状态4：已完成
class CompletedState : public OrderState {
public:
    string getName() const override { return "已完成"; }
    void pay(Order& order) override {
        cout << "错误：" << getName() << " 订单无需支付" << endl;
    }
    void ship(Order& order) override {
        cout << "错误：" << getName() << " 订单不能发货" << endl;
    }
    void complete(Order& order) override {
        cout << "错误：" << getName() << " 订单不能重复完成" << endl;
    }
};
// 订单类（上下文）
class Order{
private:
  unique_ptr<OrderState> state_;
  string order_id_;
public:
  Order(string id):order_id_(id)
  {
    // 初始状态：未支付
    state_=make_unique<UnpaidState>();
    cout << "订单 " << order_id_ << " 创建，状态：" << state_->getName() << endl;
  }
  void setState(unique_ptr<OrderState> state) {
    state_ = move(state);
    cout << "订单 " << order_id_ << " 状态更新为：" << state_->getName() << endl;
  }
  // 暴露操作接口（委托给当前状态）
  void pay() { state_->pay(*this); }
  void ship() { state_->ship(*this); }
  void complete() { state_->complete(*this); }

};
// 实现状态转换逻辑（需要访问 Order 类，所以放在 Order 定义后）
void UnpaidState::pay(Order& order) {
    order.setState(make_unique<PaidState>());  // 未支付→已支付
}

void PaidState::ship(Order& order) {
    order.setState(make_unique<ShippedState>()); // 已支付→已发货
}

void ShippedState::complete(Order& order) {
    order.setState(make_unique<CompletedState>()); // 已发货→已完成
}
int main()
{
  Order order("ORD123456");
  order.pay();    // 未支付→已支付
  order.ship();   // 已支付→已发货
  order.complete();// 已发货→已完成
  // 错误操作（已完成订单不能支付）
  order.pay();
  return 0;
}
```

