#pragma once

class ReceivedMessage;

#include "Message.h"

class ReceivedMessage
{
private:
  std::string _senderId;
  std::shared_ptr<Message> _message;
public:
  ReceivedMessage(const std::string& senderId, const std::shared_ptr<Message>& msg) : _senderId(senderId), _message(msg) { }
  const std::string& GetSenderId() const { return _senderId; }
  const std::shared_ptr<Message>& GetMessage() const { return _message; }
};