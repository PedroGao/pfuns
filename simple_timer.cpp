#include <iostream>
#include <queue>
#include <stdint.h>
#include <time.h>
#include <unistd.h>

using namespace std;

struct Task
{
  uint64_t expire;                                                          // 过期时间
  string message;                                                           // 过期消息，可以换成回调函数
  bool operator<(const Task &other) const { return expire > other.expire; } // 优先队列将 expire 最小的放到最前面
};

class Timer
{
private:
  // 优先队列
  priority_queue<Task> queue;
  // 循环时间
  int timeout_in_seconds;
  // 获得当前时间戳
  uint64_t get_ms();
  // 时间循环
  void evenloop();

public:
  Timer(int timeout_in_seconds = 0);
  ~Timer();

  void add_task(int ms, string message);

  void run();
};

Timer::Timer(int timeout_in_seconds)
{
  this->timeout_in_seconds = timeout_in_seconds;
}

// 获得当前系统时间戳
uint64_t Timer::get_ms()
{
  timespec ts;
  clock_gettime(CLOCK_MONOTONIC, &ts);
  return ts.tv_nsec / 1000000 + ts.tv_sec * 1000ull;
}

void Timer::run()
{
  evenloop();
}

void Timer::add_task(int ms, string message)
{
  queue.push(Task{.expire = get_ms() + ms * 1000, .message = message});
}

void Timer::evenloop()
{
  uint64_t start = get_ms();
  while (true)
  {
    // 睡眠 1 s
    usleep(1000);
    while (!queue.empty())
    {
      if (get_ms() > queue.top().expire)
      {
        Task task = queue.top();
        queue.pop();
        cout << "task: " << task.message << " is executed." << endl;
      }
      else
      {
        break;
      }
    }

    // 超时退出
    if ((get_ms() - start) > timeout_in_seconds * 1000)
    {
      cout << "evenloop is over" << endl;
      break;
    }
  }
}

Timer::~Timer()
{
}

int main(int argc, char const *argv[])
{
  Timer timer(10);
  timer.add_task(1, "task1");
  timer.add_task(1, "task2");
  timer.add_task(5, "task3");
  timer.add_task(2, "task4");
  timer.run();
  return 0;
}
