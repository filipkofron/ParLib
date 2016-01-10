#pragma once

#include <cstdint>
#include <set>
#include <istream>
#include <vector>

class Matrix
{
private:
  uint32_t size;
  uint8_t **array;

public:

  Matrix();
  Matrix(const Matrix &matrix);
  Matrix& operator =(const Matrix &matrix);

  void initialize(const uint32_t &size);
  Matrix(const uint32_t &size);
  ~Matrix();

  void clear();
  uint8_t &get(uint32_t x, uint32_t y);

  uint8_t get(uint32_t x, uint32_t y) const;

  void set(uint32_t x, uint32_t y, uint8_t val);

  void fromStream(std::istream &is);

  int32_t sum(std::vector<int32_t> &set) const;

  std::set<int32_t> getNeigh(int32_t idx) const;

  const uint32_t &getSize() const;

  uint8_t **getArray();
};
