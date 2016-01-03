#pragma once
#include <memory>
#include "Message.h"

class MessageFactory
{
public:
  static std::shared_ptr<Message> CreateInitialMessage(const std::string& myId);
  static std::shared_ptr<Message> CreateElectionMessage(const std::string& myId);
  static std::shared_ptr<Message> CreateElectedMessage(const std::string& myId);
  static std::shared_ptr<Message> CreateKeepAliveMessage();
  static std::shared_ptr<Message> CreateKeepAliveMessageResp();
  static std::shared_ptr<Message> CreateDataMessage(const uint8_t* data);
};
