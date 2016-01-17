#pragma once
#include <memory>

class MessageFactory;

#include "Message.h"

class MessageFactory
{
public:
  static std::shared_ptr<Message> CreateInitialMessage(const std::string& myId);
  static std::shared_ptr<Message> CreateElectionMessage(const std::string& myId);
  static std::shared_ptr<Message> CreateElectedMessage(const std::string& myId);
  static std::shared_ptr<Message> CreateKnownLeaderMessage(const std::string& myId);
  static std::shared_ptr<Message> CreateKeepAliveMessage();
  static std::shared_ptr<Message> CreateKeepAliveMessageResp();
  static std::shared_ptr<Message> CreateStackAssignmentMessage(StackAssignment& stackAssign);
  static std::shared_ptr<Message> CreateAssignmentFinishedMessage(int32_t bestFound);
  static std::shared_ptr<Message> CreateDivideWithMessage(const std::string& with);
  static std::shared_ptr<Message> CreateTerminateMessage();
};
