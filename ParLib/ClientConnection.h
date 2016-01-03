#pragma once
#include "TCPSocket.h"
#include <thread>

class ClientConnection
{
private:
  std::shared_ptr<TCPSocket> _socket;
  std::shared_ptr<std::thread> _receiverThread;
  void ReceiverLoop();
public:
  ClientConnection(const std::shared_ptr<TCPSocket>& socket);
  void StartReceiverThread();
};