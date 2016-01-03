#include "NetworkManager.h"
#include "Common.h"
#include "AddrIterator.h"
#include <iostream>

void NetworkManager::AddFoundServers(const std::vector<std::shared_ptr<TCPSocket> >& sockets)
{
  for (std::shared_ptr<TCPSocket> socket : sockets)
  {
    std::shared_ptr<Message> msg = Message::Receive(*socket);
    if (msg)
    {
      std::cout << "win!" << std::endl;
    }
  }
}

NetworkManager::NetworkManager(const std::string& network, int maskBits)
  : _network(network), _maskBits(maskBits)
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
  while (true)
  {
    std::string next = addrIterator.NextAddr();
    if (!next.size())
    {
      break;
    }
    for (int i = defaultPort; i < maxPort; i++)
    {
      std::cout << "Trying " << next << ":" << i << std::endl;
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

void NetworkManager::Terminate()
{
  _serverConnection->StopServer();
  _clientConnections.clear();
}

const std::string& NetworkManager::GetNetworkId()
{
  if (!_networkId.size())
  {
    FatalError("Network id unknown!");
  }
  return _networkId;
}
