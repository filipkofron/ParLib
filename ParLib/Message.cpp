#include "TCPSocket.h"
#include "Message.h"

std::shared_ptr<Message> Message::Receive(TCPSocket& socket)
{
  return nullptr;
}

bool Message::Send(TCPSocket& socket)
{
  return false;
}