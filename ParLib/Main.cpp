
#include "TCPSocket.h"
#include "Common.h"
#include <iostream>

int main(void)
{
  OnStart();

  TCPSocket::GetLocalAddresses();

  OnEnd();
  return 0;
}