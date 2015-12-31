#pragma once
#include "TCPSocket.h"
#include <set>
#include "ServerConnection.h"
#include "ClientConnection.h"

class NetworkManager
{
private:
  std::string _networkId;
  std::shared_ptr<ServerConnection> _serverConnection;
  std::vector<std::shared_ptr<ClientConnection> > _clientConnections;
public:
  NetworkManager(const std::string& network, int maskBits);
  void Terminate();
};
