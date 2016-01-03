#include <string>
#include "MessageFactory.h"

std::shared_ptr<Message> MessageFactory::CreateInitialMessage(const std::string& myId)
{
  Message* msg = new Message(MESSAGE_TYPE_INITIAL, myId.c_str(), myId.size());
  return std::shared_ptr<Message>(msg);
}
/*
std::shared_ptr<Message> MessageFactory::CreateElectionMessage(const std::string& myId)
{
}

std::shared_ptr<Message> MessageFactory::CreateElectedMessage(const std::string& myId)
{
}

std::shared_ptr<Message> MessageFactory::CreateKeepAliveMessage()
{
}

std::shared_ptr<Message> MessageFactory::CreateDataMessage(const uint8_t* data)
{
}*/