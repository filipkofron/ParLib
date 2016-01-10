#pragma once

class AddrSubnetIterator;

#include <string>
#include "Message.h"

class AddrIterator
{
  
};

class AddrSubnetIterator
{
private:
  int _bits;
  uint32_t _addr;
  uint32_t _stateAddr;
  uint32_t _maxAddr;
public:
  AddrSubnetIterator(const std::string& addr, int bits)
    : _bits(bits), _addr(ParseIPV4Addr(addr))
  {
    _stateAddr = _addr >> bits;
    _stateAddr = _stateAddr << bits;
    _maxAddr = 0xFFFFFFFF << (32 - bits);
    _maxAddr = _maxAddr >> (32 - bits);
    _maxAddr = _maxAddr | _addr;
  }

  virtual std::string NextAddr()
  {
    std::string res = "";
    if (_stateAddr <= _maxAddr)
    {
      uint8_t* bytes = reinterpret_cast<uint8_t*>(&_stateAddr);
      bool littleEndian = static_cast<void*>(&_stateAddr) == static_cast<void*>(bytes);
      for (int i = 0; i < 4; i++)
      {
        int endianIdx = littleEndian ? 3 - i : i;
        res += std::to_string(static_cast<int>(bytes[endianIdx]));
        if (i < 3)
        {
          res += ".";
        }
      }
      _stateAddr++;
    }
      
    return res;
  }
};

class AddrRangeIterator
{
  
};