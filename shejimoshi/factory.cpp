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