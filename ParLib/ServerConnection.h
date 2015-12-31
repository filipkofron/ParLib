#pragma once
#include "TCPSocket.h"
#include <vector>
#include <mutex>

class ServerConnection
{
private:
  std::shared_ptr<TCPSocket> _listenSocket;
  std::vector<std::shared_ptr<TCPSocket> > _connectedClients;
  std::mutex _lock;
  std::shared_ptr<std::thread> _listenThread;
  int _port;
public:
  bool InitServer();
  bool StartServer();
  void StopServer();
  void DisconnectClients();
  static void AcceptLoop(ServerConnection* instance);
  int GetPort() const { return _port; }
};
