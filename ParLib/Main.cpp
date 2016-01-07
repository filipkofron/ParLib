
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
  //std::string ip = "147.32.76.241";
  std::string ip = "192.168.1.1";
  GNetworkManager = std::make_shared<NetworkManager>(ip, 8);
  sleepMs(500);
  GNetworkManager->DiscoverAll();
  sleepMs(1000);
  //DEBUGVerbose = true;
  while (GNetworkManager->GetClientCount() < 4)
  // while (GNetworkManager->GetClientCount())
  {
    sleepMs(3000);
    std::cout << "CONNECTED CLIENTS: " << GNetworkManager->GetClientCount() << std::endl;
  }

  std::cout << "Press any key to test the server." << std::endl;
  std::cin.get();

  std::cout << "Press any key to terminate the server." << std::endl;
  std::cin.get();
  GNetworkManager->Terminate();
  GNetworkManager = nullptr;

  OnEnd();
  return 0;
}
