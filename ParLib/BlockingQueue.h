#pragma once
#include <mutex>
#include <queue>

template<typename T>
class BlockingQueue
{
private:
  std::queue<T> _items;
  std::mutex _lock;
  std::condition_variable _cond;
public:
  T PopNonBlock()
  {
    std::lock_guard<std::mutex> guard(_lock);
    if (_items.size() > 0)
    {
      T top = _items.front();
      _items.pop();
      return top;
    }
    return T();
  }
  T Pop()
  {
    std::unique_lock<std::mutex> lock(_lock);
    if (_items.size() == 0)
    {
      _cond.wait(lock);
    }
    
    if (_items.size() > 0)
    {
      T top = _items.front();
      _items.pop();
      return top;
    }
    return T();
  }
  void Push(const T& item)
  {
    std::lock_guard<std::mutex> guard(_lock);
    _items.push(item);
    _cond.notify_one();
  }
  size_t IsEmpty()
  {
    std::lock_guard<std::mutex> guard(_lock);
    return _items.size() == 0;
  }
};