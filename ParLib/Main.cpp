
#include "TCPSocket.h"
#include "Common.h"
#include <iostream>
#include "NetworkManager.h"

std::shared_ptr<NetworkManager> GNetworkManager;

int main(void)
{
  OnStart();

  GNetworkManager = std::make_shared<NetworkManager>("192.168.1.5", 8);

  std::cout << "Press any key to terminate the server." << std::endl;
  std::cin.get();

  GNetworkManager->Terminate();
  OnEnd();
  return 0;
}