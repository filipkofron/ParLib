#include "ServerConnection.h"
#include "Common.h"

bool ServerConnection::StartServer()
{
  std::lock_guard<std::mutex> guard(_lock);
  _listenSocket = std::make_shared<TCPSocket>("0.0.0.0", DEFAULT_PORT, false);
  std::thread(AcceptLoop, this);
  return _listenSocket->IsOk();
}

void ServerConnection::StopServer()
{
  std::lock_guard<std::mutex> guard(_lock);
  _listenSocket = nullptr;
}

void ServerConnection::DisconnectClients()
{
  std::lock_guard<std::mutex> guard(_lock);
  _connectedClients.clear();
}

void ServerConnection::AcceptLoop(ServerConnection* instance)
{
  auto listenSocket = instance->_listenSocket;
  while (instance->_listenSocket && listenSocket == instance->_listenSocket)
  {
    auto clientSocket = listenSocket->Accept();
    std::lock_guard<std::mutex> guard(instance->_lock);
    if (clientSocket)
    {
      instance->_connectedClients.push_back(clientSocket);
    }
  }
}