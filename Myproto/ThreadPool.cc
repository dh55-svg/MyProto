#include <mutex>
#include <condition_variable>
#include <queue>
#include <vector>
#include <thread>
#include <functional>
#include <memory>

class ThreadPool {
public:
  using Task = std::function<void()>;

  // 构造函数：指定线程数和可选的初始化回调
  explicit ThreadPool(size_t threadNum, Task initCallback = nullptr)
      : threadNum_(threadNum), running_(false), threadInitCallback_(std::move(initCallback)) {}

  ~ThreadPool() {
    if (running_) stop();  // 析构时自动停止
  }

  // 启动线程池（创建 worker 线程）
  void start() {
    if (running_) return;
    running_ = true;
    workers_.reserve(threadNum_);
    for (size_t i = 0; i < threadNum_; ++i) {
      workers_.emplace_back([this]() { workerLoop(); });  // 每个线程绑定 workerLoop
    }
  }

  // 提交任务（右值引用版本，支持完美转发）
  template <typename F>
  void addTask(F&& task) {
    std::lock_guard<std::mutex> lock(mtx_);
    queue_.emplace(std::forward<F>(task));
    cv_.notify_one();  // 唤醒一个 worker 线程
  }

  // 停止线程池（等待所有任务执行完毕并回收线程）
  void stop() {
    {
      std::lock_guard<std::mutex> lock(mtx_);
      running_ = false;
    }
    cv_.notify_all();  // 唤醒所有线程
    for (auto& worker : workers_) {
      if (worker.joinable()) worker.join();
    }
    workers_.clear();
  }

  // 禁止拷贝和移动（线程池不可复制）
  ThreadPool(const ThreadPool&) = delete;
  ThreadPool& operator=(const ThreadPool&) = delete;

private:
  // Worker 线程循环函数
  void workerLoop() {
    if (threadInitCallback_) {
      threadInitCallback_();  // 执行初始化回调
    }

    while (running_) {  // 循环处理任务，直到 running_ 为 false
      Task task;
      // 加锁获取任务
      std::unique_lock<std::mutex> lock(mtx_);
      // 等待条件：队列非空 或 线程池停止
      cv_.wait(lock, [this]() { return !running_ || !queue_.empty(); });

      if (!running_ && queue_.empty()) break;  // 停止且无任务，退出

      if (!queue_.empty()) {
        task = std::move(queue_.front());  // 任务所有权转移
        queue_.pop();
      }

      lock.unlock();  // 解锁后执行任务，避免长时间持有锁
      if (task) {
        task();  // 执行任务
      }
    }
  }

private:
  size_t threadNum_;                  // worker 线程数量
  bool running_;                      // 线程池运行状态
  std::vector<std::thread> workers_;  // worker 线程集合
  std::queue<Task> queue_;            // 任务队列
  std::mutex mtx_;                    // 保护队列和 running_ 的互斥锁
  std::condition_variable cv_;        // 线程间通知的条件变量
  Task threadInitCallback_;           // 线程初始化回调（可选）
};