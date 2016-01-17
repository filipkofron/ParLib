#pragma once

#include <vector>
#include <memory>
#include <stdio.h>

template <class T> class ParallelStack
{
private:
  std::vector<T> data;
public:
  ParallelStack();
  bool empty() const;
  void push(const T& item);
  T& top();
  void pop();
  size_t size();
  void assignData(const std::vector<T>& data);
  std::vector<T> getData();
  ParallelStack<T> giveTopHalf();
  ParallelStack<T> giveTopQuarter();
  std::shared_ptr<ParallelStack<T> > giveInterleaved();
};


template<class T> ParallelStack<T>::ParallelStack()
{
}

template<class T> bool ParallelStack<T>::empty() const
{
  return data.empty();
}

template<class T> void ParallelStack<T>::push(const T& item)
{
  data.push_back(item);
}

template<class T> T& ParallelStack<T>::top()
{
  return data.back();
}

template<class T> void ParallelStack<T>::pop()
{
  data.pop_back();
}

template<class T>
size_t ParallelStack<T>::size()
{
  return data.size();
}

template<class T> void ParallelStack<T>::assignData(const std::vector<T>& data)
{
  this->data = data;
}

template<class T>
std::vector<T> ParallelStack<T>::getData()
{
  return data;
}

template<class T> ParallelStack<T> ParallelStack<T>::giveTopHalf()
{
  ParallelStack<T> top;

  std::vector<T> newData(data.size() - data.size() / 2);

  for (size_t i = 0; i < newData.size(); i++)
  {
    newData[i] = data[i + data.size() / 2];
  }

  data.resize(data.size() / 2);

  top.assignData(newData);

  return top;
}

template<class T> ParallelStack<T> ParallelStack<T>::giveTopQuarter()
{
  ParallelStack<T> top;

  int rest = data.size() - data.size() / 4;
  if (rest == data.size())
  {
    rest--;
  }

  std::vector<T> newData(rest);

  for (size_t i = 0; i < rest; i++)
  {
    newData[i] = data[i + data.size() - rest];
  }

  data.resize(data.size() / 4);
  top.assignData(data);
  assignData(newData);
  return top;
}

template<class T>
std::shared_ptr<ParallelStack<T> > ParallelStack<T>::giveInterleaved()
{
  std::shared_ptr<ParallelStack<T> > given = std::make_shared<ParallelStack<T> >();

  std::vector<T> newDataA(data.size() / 2);
  std::vector<T> newDataB(data.size() - data.size() / 2);


  for (size_t i = 0; i < newDataA.size(); i++)
  {
    newDataA[i] = data[i * 2 + 1];
  }

  for (size_t i = 0; i < newDataB.size(); i++)
  {
    newDataB[i] = data[i * 2];
  }

  assignData(newDataB);
  given->assignData(newDataA);

  return given;
}
