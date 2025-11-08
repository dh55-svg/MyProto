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