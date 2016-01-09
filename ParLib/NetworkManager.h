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

  std::shared_ptr<ServerConnection> _serverConnection;
  std::vector<std::shared_ptr<ClientConnection> > _finishingClients;
  std::unordered_map<std::string, std::shared_ptr<ClientConnection> > _clientConnections;

  mutable std::mutex _lock;
  mutable std::mutex _finishingLock;

  std::string _networkId;

  bool _terminate;
  bool _discovered;
  std::thread _keepAliveThread;

  std::thread _mainLoopThread;

  std::string _leaderId;
  bool _leader;
  int64_t _sentElectionTime;
  bool _electionParticipant;
  
  void AddFoundServers(const std::vector<std::shared_ptr<TCPSocket> >& sockets);
  bool CheckForServer(TCPSocket* socket);
  void CleanFinishingClients();
  void OnKeepAliveMessage(const std::shared_ptr<ReceivedMessage>& msg);
  void OnElectionMessage(const std::shared_ptr<ReceivedMessage>& msg);
  void OnElectedMessage(const std::shared_ptr<ReceivedMessage>& msg);
  void MainLoop();
  void Step();
public:
  NetworkManager(const std::string& network, int maskBits);
  std::shared_ptr<ClientConnection> FindClient(const std::string& networkId);
  void DiscoverAll();
  void KeepAliveLoop();
  void DisconnectClients();
  void Terminate();
  void RegisterFinishingClient(std::shared_ptr<ClientConnection> client);
  void OnMessage(const std::shared_ptr<ReceivedMessage>& msg);
  bool SendMessage(const Message& msg, const std::string& destId);
  bool AddOrDiscardClient(const std::shared_ptr<ClientConnection>& client, bool isClient);
  void DiscardClient(ClientConnection* client);
  const std::string& GetNetworkId() const;
  std::string GetNextId(const std::string& prev) const;
  int GetClientCount() const { return _clientConnections.size(); }
};
