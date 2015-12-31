#include "NetworkManager.h"
#include "Common.h"

NetworkManager::NetworkManager(const std::string& network, int maskBits)
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

void NetworkManager::Terminate()
{
  _serverConnection->StopServer();
  _clientConnections.clear();
}
