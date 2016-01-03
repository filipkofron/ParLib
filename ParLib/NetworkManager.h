#pragma once
#include "TCPSocket.h"
#include "ServerConnection.h"
#include "ClientConnection.h"
#include <unordered_map>

class NetworkManager
{
private:
  std::string _network;
  int _maskBits;
  std::string _networkId;
  std::shared_ptr<ServerConnection> _serverConnection;
  std::unordered_map<std::string, std::shared_ptr<ClientConnection> > _clientConnections;
  void AddFoundServers(const std::vector<std::shared_ptr<TCPSocket> >& sockets);
  bool CheckForServer(TCPSocket* socket);
public:
  NetworkManager(const std::string& network, int maskBits);
  void DiscoverAll();
  void Terminate();
  const std::string& GetNetworkId();
};
