#include <string>
#include "MessageFactory.h"

std::shared_ptr<Message> MessageFactory::CreateInitialMessage(const std::string& myId)
{
  Message* msg = new Message(MESSAGE_TYPE_INITIAL, myId.c_str(), myId.size());
  return std::shared_ptr<Message>(msg);
}

std::shared_ptr<Message> MessageFactory::CreateElectionMessage(const std::string& myId)
{
  Message* msg = new Message(MESSAGE_TYPE_ELECTION, myId.c_str(), myId.size());
  return std::shared_ptr<Message>(msg);
}

std::shared_ptr<Message> MessageFactory::CreateElectedMessage(const std::string& myId)
{
  Message* msg = new Message(MESSAGE_TYPE_ELECTED, myId.c_str(), myId.size());
  return std::shared_ptr<Message>(msg);
}

std::shared_ptr<Message> MessageFactory::CreateKnownLeaderMessage(const std::string& myId)
{
  Message* msg = new Message(MESSAGE_TYPE_KNOWN_LEADER, myId.c_str(), myId.size());
  return std::shared_ptr<Message>(msg);
}

std::shared_ptr<Message> MessageFactory::CreateKeepAliveMessage()
{
  Message* msg = new Message(MESSAGE_TYPE_KEEP_ALIVE, "", 0);
  return std::shared_ptr<Message>(msg);
}

std::shared_ptr<Message> MessageFactory::CreateKeepAliveMessageResp()
{
  Message* msg = new Message(MESSAGE_TYPE_KEEP_ALIVE_RESP, "", 0);
  return std::shared_ptr<Message>(msg);
}

std::shared_ptr<Message> MessageFactory::CreateStackAssignmentMessage(StackAssignment& stackAssign)
{
  ByteOutputStream bos;
  stackAssign.Serialize(bos);
  Message* msg = new Message(MESSAGE_TYPE_STACK_ASSIGNMENT, reinterpret_cast<const char*>(bos.GetData()), bos.GetLength());
  return std::shared_ptr<Message>(msg);
}

std::shared_ptr<Message> MessageFactory::CreateAssignmentFinishedMessage(int32_t bestFound)
{
  ByteOutputStream bos;
  bos.PutInt32(bestFound);
  Message* msg = new Message(MESSAGE_TYPE_ASSIGNMENT_FINISHED, reinterpret_cast<const char*>(bos.GetData()), bos.GetLength());
  return std::shared_ptr<Message>(msg);
}
