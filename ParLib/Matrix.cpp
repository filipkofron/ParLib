#include "Matrix.h"
#include <iostream>
#include <algorithm>

Matrix::Matrix()
  : size(0), array(nullptr)
{
}

Matrix::Matrix(const Matrix &matrix)
  : size(0), array(nullptr)
{
  *this = matrix;
}

Matrix::~Matrix()
{
  if (this->size > 0)
  {
    clear();
  }
}

Matrix& Matrix::operator= (const Matrix &matrix)
{
  if (this == &matrix)
  {
    return *this;
  }
  clear();
  if (matrix.size > 0)
  {
    initialize(matrix.size);
    for (uint32_t i = 0; i < size; i++)
    {
      for (uint32_t j = 0; j < size; j++)
      {
        array[i][j] = matrix.array[i][j];
      }
    }
  }
  return *this;
}

void Matrix::clear()
{
  if (size)
  {
    if (array)
    {
      for (uint32_t i = 0; i < size; i++)
      {
        delete[] array[i];
      }
    }
    delete[] array;
  }
}

void Matrix::initialize(const uint32_t &size)
{
  clear();
  this->size = size;
  array = new uint8_t *[size];
  for (uint32_t i = 0; i < size; i++)
  {
    array[i] = new uint8_t[size];
  }
}

Matrix::Matrix(const uint32_t &size)
{
  initialize(size);
}

uint8_t &Matrix::get(uint32_t x, uint32_t y)
{
  return array[x][y];
}

uint8_t Matrix::get(uint32_t x, uint32_t y) const
{
  return array[x][y];
}

void Matrix::set(uint32_t x, uint32_t y, uint8_t val)
{
  array[x][y] = val;
}

void Matrix::fromStream(std::istream &is)
{
  is >> size;

  initialize(size);

  for (uint32_t i = 0; i < size; i++)
  {
    for (uint32_t j = 0; j < size; j++)
    {
      int val;
      is >> val;
      array[i][j] = static_cast<uint8_t>(val);
    }
  }
#ifdef debug
  Log << "Loaded matrix of side " << size << std::endl;
#endif

  /*for (uint32_t i = 0; i < size; i++)
  {
  for (uint32_t j = 0; j < size; j++)
  {
  std::cout<<array[i][j]<<" ";
  }
  std::cout<<std::endl;
  }*/
}

int32_t Matrix::sum(std::vector<int32_t> &vertSet) const
{
  uint32_t mySum = 0;
  //uint32_t i = 0;
#ifdef debug
  Log << " computing set:";
  for (auto t : vertSet)
  {
    Log << t << " ";
  }
  Log << std::endl;
  std::cin.get();
#endif
  int *vertTemp = new int[size];
  for (auto t : vertSet)
  {
    for (uint32_t j = 0; j < size; j++)
    {
      vertTemp[j] = 1;
    }
    for (uint32_t i : vertSet)
    {
      vertTemp[i] = 0;
    }
    for (uint32_t j = 0; j < size; j++)
    {
      if (vertTemp[j])
      {
        mySum += array[t][j];
      }
    }
    /*for (uint32_t j = 0; j < size; j++)
    {
    if (std::find(vertSet.begin(), vertSet.end(), j) == vertSet.end())
    {
    mySum += array[t][j];

    #ifdef debug
    std::cout << "mySum:" << mySum;
    std::cout << std::endl;
    std::cin.get();
    #endif
    }
    }*/
  }

  delete[] vertTemp;

  return mySum;
}

std::set<int32_t> Matrix::getNeigh(int32_t idx) const
{
  std::set<int32_t> ret;

  for (uint32_t i = 0;i<size;i++)
  {
    if (array[idx][i]>0)
    {
      ret.insert(i);
    }
  }

  return ret;

}

const uint32_t &Matrix::getSize() const
{
  return size;
}

uint8_t **Matrix::getArray()
{
  return array;
}
