#pragma once

class NetworkManager;

#include "TCPSocket.h"
#include "ServerConnection.h"
#include "ClientConnection.h"
#include <unordered_map>
#include "ReceivedMessage.h"

class NetworkManager
{
private:
  std::string _network;
  int _maskBits;
  std::mutex _lock;
  std::string _networkId;
  std::string _leaderId;
  std::shared_ptr<ServerConnection> _serverConnection;
  std::vector<std::shared_ptr<ClientConnection> > _finishingClients;
  std::unordered_map<std::string, std::shared_ptr<ClientConnection> > _clientConnections;
  void AddFoundServers(const std::vector<std::shared_ptr<TCPSocket> >& sockets);
  bool CheckForServer(TCPSocket* socket);
  void CleanFinishingClients();
public:
  NetworkManager(const std::string& network, int maskBits);
  void DiscoverAll();
  void DisconnectClients();
  void Terminate();
  void RegisterFinishingClient(std::shared_ptr<ClientConnection> client);
  void OnMessage(const std::shared_ptr<ReceivedMessage>& msg);
  void AddOrDiscardClient(const std::shared_ptr<ClientConnection>& client);
  const std::string& GetNetworkId();
};
