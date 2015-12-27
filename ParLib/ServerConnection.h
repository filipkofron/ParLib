#pragma once
#include "TCPSocket.h"
#include <vector>

class ServerConnection
{
private:
  std::unique_ptr<TCPSocket> _listenSocket;
  std::vector<std::unique_ptr<TCPSocket> > _connectedClients;
public:
  bool StartServer();
  void DisconnectClient();
};