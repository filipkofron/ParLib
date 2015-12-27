#pragma once
#include "TCPSocket.h"
#include <set>
#include "ServerConnection.h"
#include "ClientConnection.h"

class NetworkManager
{
private:
  std::unique_ptr<ServerConnection> _serverConnection;
  std::unique_ptr<ClientConnection> _clientConnection;
public:
  void Init();
};
