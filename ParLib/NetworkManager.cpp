#include "NetworkManager.h"
#include "Common.h"
#include "AddrIterator.h"
#include <iostream>

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
  Sleep(1000);
}

void NetworkManager::DiscoverAll()
{
  std::cout << "Discovering all nodes on the network " << _network << ". " << (1 << _maskBits) << " hosts will be scanned." << std::endl;
  int defaultPort = DEFAULT_PORT;
  int maxPort = defaultPort + 8;
  AddrIterator addrIterator(_network, _maskBits);
  std::vector<TCPSocket*> foundSockets;
  while (true)
  {
    std::string next = addrIterator.NextAddr();
    if (!next.size())
    {
      break;
    }
    for (int i = defaultPort; i < maxPort; i++)
    {
      if (i == 1337 && next == "192.168.1.161")
      {
        std::cout << "test" << std::endl;
      }
      else
      {
        continue;
      }
      std::cout << "Trying " << next << ":" << i << std::endl;
      TCPSocket* socket = new TCPSocket(next, i, true, 500);
      char b = 0;
      if (socket->IsOk())
      {
        std::cout << "test" << std::endl;
      }
      socket->SetTimeout(5000);
      int rec = socket->Receive(&b, 1);
      Error("Receiving error: '%s'\n", strerror(errno));
      if (socket->IsOk() && rec == 1 && b == 66)
      {
        socket->ResetTimeouts();
        foundSockets.push_back(socket);
      }
      else
      {
        delete socket;
      }
    }
  }
}

void NetworkManager::Terminate()
{
  _serverConnection->StopServer();
  _clientConnections.clear();
}
