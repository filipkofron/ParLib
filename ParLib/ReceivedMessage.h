#pragma once

#include "Message.h"

class ReceivedMessage : public Message
{
private:
  std::string _senderId;
public:
  ReceivedMessage(const std::string& senderId) : _senderId(senderId) { }

  const std::string& GetSenderId() const { return _senderId; }
};