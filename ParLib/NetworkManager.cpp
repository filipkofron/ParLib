#include "NetworkManager.h"
#include "Common.h"
#include "AddrIterator.h"
#include <iostream>
#include "MessageFactory.h"

void NetworkManager::AddFoundServers(const std::vector<std::shared_ptr<TCPSocket> >& sockets)
{
  for (auto& socket : sockets)
  {
    auto client = std::make_shared<ClientConnection>(socket);
    client->StartReceiverThread(client, true);
  }
}

NetworkManager::NetworkManager(const std::string& network, int maskBits)
  : _network(network), _maskBits(maskBits), _keepAliveLoop(true), _keepAliveThread(&NetworkManager::KeepAliveLoop, this)
{
  std::string localAddr = TCPSocket::GetLocalAddressInSubnet(network, maskBits);
  _serverConnection = std::make_shared<ServerConnection>();
  if (!_serverConnection->InitServer())
  {
    FatalError("Cannot initialize the NetworkManager's server.");
  }
  _networkId = localAddr + "_" + std::to_string(_serverConnection->GetPort());
  if (!_serverConnection->StartServer())
  {
    FatalError("Cannot start accepting clients on the NetworkManager's server.");
  }
}

std::shared_ptr<ClientConnection> NetworkManager::FindClient(const std::string networkId)
{
  std::lock_guard<std::mutex> guard(_lock);
  std::shared_ptr<ClientConnection> conn(nullptr);
  if (_clientConnections.find(networkId) != _clientConnections.end())
  {
    conn = _clientConnections[networkId];
  }
  return conn;
}

void NetworkManager::CleanFinishingClients()
{
  std::lock_guard<std::mutex> guard(_finishingLock);
  for (auto client : _finishingClients)
  {
    client->CleanUp();
  }
  _finishingClients.clear();
}

void NetworkManager::OnKeepAliveMessage(const std::shared_ptr<ReceivedMessage>& msg)
{
  auto conn = FindClient(msg->GetSenderId());

  if (!conn)
    return;

  conn->SendKeepAliveResp();
}

bool NetworkManager::CheckForServer(TCPSocket* socket)
{
  char b = 0;
  socket->SetTimeout(5);
  int rec = socket->Receive(&b, 1);
  return socket->IsOk() && rec == 1 && b == 66;
}

void NetworkManager::DiscoverAll()
{
  sleepMs(1000);
  std::cout << "Discovering all nodes on the network " << _network << ". " << (1 << _maskBits) << " hosts will be scanned." << std::endl;
  int defaultPort = DEFAULT_PORT;
  int maxPort = defaultPort + 8;
  AddrIterator addrIterator(_network, _maskBits);
  std::vector<std::shared_ptr<TCPSocket>  > foundSockets;
#ifndef _WIN32
  const int maxTries = 16;
#endif // _WIN32
  int tries = 0;
  while (true)
  {
#ifndef _WIN32
    if (tries > maxTries)
    {
      break;
    }
#endif // _WIN32
    tries++;
    std::string next = addrIterator.NextAddr();
    if (!next.size())
    {
      break;
    }
    for (int i = defaultPort; i < maxPort; i++)
    {
      // std::cout << "Trying " << next << ":" << i << std::endl;
      TCPSocket* socket = new TCPSocket(next, i, true, 5);
      if (CheckForServer(socket))
      {
        socket->ResetTimeouts();
        foundSockets.push_back(std::shared_ptr<TCPSocket> (socket));
      }
      else
      {
        delete socket;
      }
    }
  }
  AddFoundServers(foundSockets);
}

void NetworkManager::KeepAliveLoop()
{
  while (_keepAliveLoop)
  {
    sleepMs(1000);
    CleanFinishingClients();
   // std::cout << "Sending keep alive messages!" << std::endl;
    std::lock_guard<std::mutex> guard(_lock);
    for (auto& client : _clientConnections)
    {
      std::cout << "Sending keep alive message to: " << client.first << std::endl;
      client.second->SendKeepAlive();
    }
  }
}

void NetworkManager::DisconnectClients()
{
  CleanFinishingClients();
  std::lock_guard<std::mutex> guard(_lock);
  _clientConnections.clear();
}

void NetworkManager::Terminate()
{
  _keepAliveLoop = false;
  CleanFinishingClients();
  _serverConnection->StopServer();

  _keepAliveThread.join();
  std::lock_guard<std::mutex> guard(_lock);
  _clientConnections.clear();
}

void NetworkManager::RegisterFinishingClient(std::shared_ptr<ClientConnection> client)
{
  CleanFinishingClients();
  std::lock_guard<std::mutex> guard(_finishingLock);
  _finishingClients.push_back(client);
}

void NetworkManager::OnMessage(const std::shared_ptr<ReceivedMessage>& msg)
{
  std::cout << "Received message type: " << msg->GetMessage()->GetType() << std::endl;
  switch (msg->GetMessage()->GetType())
  {
  case MESSAGE_TYPE_KEEP_ALIVE:
    OnKeepAliveMessage(msg);
  }
}

bool NetworkManager::AddOrDiscardClient(const std::shared_ptr<ClientConnection>& client, bool isClient)
{
  if (client->GetNetworkId() == GetNetworkId())
    return false;

  std::lock_guard<std::mutex> guard(_lock);
  if (_clientConnections.find(client->GetNetworkId()) == _clientConnections.end())
  {
    _clientConnections[client->GetNetworkId()] = client;
    return true;
  }
  if ((client->GetNetworkId() < GetNetworkId()) == isClient)
  {
    _clientConnections[client->GetNetworkId()] = client;
    return true;
  }
  return false;
}

void NetworkManager::DiscardClient(ClientConnection* client)
{
  std::lock_guard<std::mutex> guard(_lock);
  if (_clientConnections.find(client->GetNetworkId()) == _clientConnections.end())
    return;
  if (_clientConnections[client->GetNetworkId()].get() == client)
    _clientConnections.erase(client->GetNetworkId());
}

const std::string& NetworkManager::GetNetworkId()
{
  if (!_networkId.size())
  {
    FatalError("Network id unknown!");
  }
  return _networkId;
}
