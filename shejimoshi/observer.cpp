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