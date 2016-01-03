
#include <iostream>
#include <cstring>
#include "TCPSocket.h"
#include "Common.h"
#include "NetworkManager.h"
#include "AddrIterator.h"

std::shared_ptr<NetworkManager> GNetworkManager;

int main(int argc, const char* args[])
{
  OnStart();

  GNetworkManager = std::make_shared<NetworkManager>("192.168.1.5", 8);
  GNetworkManager->DiscoverAll();

  std::cout << "Press any key to test the server." << std::endl;
  std::cin.get();

  std::cout << "Press any key to terminate the server." << std::endl;
  std::cin.get();
  GNetworkManager->Terminate();
  GNetworkManager = nullptr;

  OnEnd();
  return 0;
}
