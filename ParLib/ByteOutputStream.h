#pragma once
#include <vector>

class ByteOutputStream
{
private:
  std::vector<uint8_t> _data;
public:
  uint8_t* GetData() { return _data.data(); }
  size_t GetLength() { return _data.size(); }
  void PutInt32(int32_t num)
  {
    uint8_t* raw = reinterpret_cast<uint8_t*>(&num);
    for (int i = 0; i < 4; i++)
    {
      _data.push_back(raw[i]);
    }
  }
  void PutInt64(int64_t num)
  {
    uint8_t* raw = reinterpret_cast<uint8_t*>(&num);
    for (int i = 0; i < 8; i++)
    {
      _data.push_back(raw[i]);
    }
  }
  void PutString(const std::string& str)
  {
    const uint8_t* raw = reinterpret_cast<const uint8_t*>(str.c_str());
    PutInt32(static_cast<int32_t>(str.size()));
    for (int i = 0; i < str.size(); i++)
    {
      _data.push_back(raw[i]);
    }
  }

  void PutIntArray(const std::vector<int>& arr)
  {
    PutInt32(static_cast<int32_t>(arr.size()));
    for (int i = 0; i < arr.size(); i++)
    {
      PutInt32(arr[i]);
    }
  }

  void PutIntArrayArray(const std::vector<std::vector<int> >& arr)
  {
    PutInt32(static_cast<int32_t>(arr.size()));
    for (int i = 0; i < arr.size(); i++)
    {
      PutIntArray(arr[i]);
    }
  }
};
