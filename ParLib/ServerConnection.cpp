#include "ServerConnection.h"
#include "Common.h"

#include <cstdlib>
#include <cerrno>
#include <cstring>

bool ServerConnection::StartServer()
{
  if (_listenThread || !_listenSocket->IsOk())
  {
    return false;
  }
  _listenThread = std::make_shared<std::thread>(AcceptLoop, this);
  return _listenThread != nullptr;
}

bool ServerConnection::InitServer()
{
  std::lock_guard<std::mutex> guard(_lock);
  int port = DEFAULT_PORT;
  while (port < 65536)
  {
    _listenSocket = std::make_shared<TCPSocket>("0.0.0.0", port, false);
    if (!_listenSocket->IsOk())
    {
      port++;
    }
    else
    {
      _port = port;
      break;
    }
  }
  if (port == 65536)
  {
    Error("Cannot bind to any port at all.");
    return false;
  }
  return _listenSocket->IsOk();
}

void ServerConnection::StopServer()
{
  {
    std::lock_guard<std::mutex> guard(_lock);
    _listenSocket = nullptr;
  }
  if (_listenThread)
  {
    _listenThread->join();
  }
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
      char b = 66;
      clientSocket->Send(&b, 1);
      std::shared_ptr<ClientConnection> conn = std::make_shared<ClientConnection>(clientSocket);
      conn->StartReceiverThread();
      instance->_connectedClients.push_back(conn);;
    }
  }
}
