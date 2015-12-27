#pragma once
#include "TCPSocket.h"

class ClientConnection
{
private:
  std::unique_ptr<TCPSocket> _socket;
public:
};