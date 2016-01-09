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

/*

std::shared_ptr<Message> MessageFactory::CreateDataMessage(const uint8_t* data)
{
}*/