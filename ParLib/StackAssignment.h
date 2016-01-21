#pragma once
#include <iostream>

class StackAssignment;

#include "Common.h"
#include "NetworkManager.h"
#include "DataMessage.h"
#include <unordered_map>
#include "ParallelStack.h"

class NetworkManager;
extern std::shared_ptr<NetworkManager> GNetworkManager;

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

  std::vector<std::string> GetClients()
  {
    std::vector<std::string> clients;
    for (auto& each : _assignments)
    {
      clients.push_back(each.first);
    }
    return clients;
  }

  size_t GetSize() const { return _assignments.size();  }

  bool Equals(const std::string& id, const std::shared_ptr<StackAssignment>& assign)
  {
    return _assignments[id] == assign->_assignments[id];
  }

  void Update(const std::shared_ptr<StackAssignment>& assign)
  {
    for (auto& each : assign->_assignments)
    {
      _assignments[each.first] = each.second;
    }
  }

  void Remove(const std::string& client)
  {
    _assignments.erase(client);
  }

  void PrintAssignments()
  {
    for (auto& each : _assignments)
    {
      Log << each.first << ": " << each.second.size() << std::endl;
    }
  }
};
