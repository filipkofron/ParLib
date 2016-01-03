#pragma once
#include <memory>
#include "Message.h"

class MessageFactory
{
public:
  std::shared_ptr<Message> CreateInitialMessage(const std::string& myId);
  std::shared_ptr<Message> CreateElectionMessage(const std::string& myId);
  std::shared_ptr<Message> CreateElectedMessage(const std::string& myId);
  std::shared_ptr<Message> CreateKeepAliveMessage();
  std::shared_ptr<Message> CreateDataMessage(const uint8_t* data);
};
