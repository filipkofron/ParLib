#pragma once
#include <cstdint>
#include <string>
#include <vector>

class ByteInputStream
{
private:
  uint8_t* _data;
  uint8_t* _curr;
public:
  ByteInputStream(uint8_t* data) : _data(data), _curr(_data) { }

  int64_t NextInt64()
  {
    int64_t num = 0;
    uint8_t* raw = reinterpret_cast<uint8_t*>(&num);
    for (int i = 0; i < 8; i++)
    {
      raw[i] = _curr[i];
    }
    _curr += 8;
    return num;
  }
  int32_t NextInt32()
  {
    int32_t num = 0;
    uint8_t* raw = reinterpret_cast<uint8_t*>(&num);
    for (int i = 0; i < 4; i++)
    {
      raw[i] = _curr[i];
    }
    _curr += 4;
    return num;
  }
  std::string NextString()
  {
    int32_t len = NextInt32();
    uint8_t* str = static_cast<uint8_t*>(malloc(len + 1));
    for (int i = 0; i < len; i++)
    {
      str[i] = _curr[i];
    }
    _curr += len;
    str[len] = '\0';
    std::string ret = reinterpret_cast<char*>(str);
    free(str);
    return ret;
  }
  std::vector<int> NextIntArray()
  {
    int32_t len = NextInt32();
    std::vector<int> ret;
    for (int i = 0; i < len; i++)
    {
      ret.push_back(NextInt32());
    }
    return ret;
  }

  std::vector<std::vector<int> > NextIntArrayArray()
  {
    int32_t len = NextInt32();
    std::vector<std::vector<int> > ret;
    for (int i = 0; i < len; i++)
    {
      ret.push_back(NextIntArray());
    }
    return ret;
  }
};
