#pragma once
#include "DataMessage.h"
#include <unordered_map>
#include "Parallelstack.h"

class StackAssignment : public DataMessage
{
private:
  std::unordered_map<std::string, std::vector<std::vector<int> > > _assignments;

public:
  void Serialize(ByteOutputStream& bos) override
  {
    int32_t count = static_cast<int32_t>(_assignments.size());
    bos.PutInt32(count);
    for (auto& each : _assignments)
    {
      bos.PutString(each.first);
      bos.PutIntArrayArray(each.second);
    }
  }

  void Deserialize(ByteInputStream& bis) override
  {
    int count = bis.NextInt32();
    for (int i = 0; i < count; i++)
    {
      std::string name = bis.NextString();
      _assignments[name] = bis.NextIntArrayArray();
    }
  }

  std::vector<std::vector<int> >& GetAssignment(const std::string& client)
  {
    return _assignments[client];
  }

  size_t GetSize() const { return _assignments.size();  }
};