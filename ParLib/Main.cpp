
#include "TCPSocket.h"
#include "Common.h"
#include <iostream>
#include "NetworkManager.h"

std::shared_ptr<NetworkManager> GNetworkManager;

int main(void)
{
  OnStart();

  GNetworkManager = std::make_shared<NetworkManager>("192.168.1.5", 8);

  std::cout << "Press any key to test the server." << std::endl;
  std::cin.get();

  TCPSocket test("halt.cz", 1234, true);
  const char* msg = "Hello from C++!";
  char buffer[16];
  test.Send(msg, strlen(msg));
  int bytes = test.Receive(buffer, 15);
  buffer[15] = '\0';
  std::cout << "Received: " << buffer << std::endl;

  std::cout << "Press any key to terminate the server." << std::endl;
  std::cin.get();

  GNetworkManager->Terminate();
  OnEnd();
  return 0;
}
