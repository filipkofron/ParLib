#pragma once
#include <vector>

class DataMessage;

#include "ByteOutputStream.h"
#include "ByteInputStream.h"

class DataMessage
{
public:
  virtual ~DataMessage()
  {
  }

  virtual void Serialize(ByteOutputStream& bos) = 0;
  virtual void Deserialize(ByteInputStream& bis) = 0;
};