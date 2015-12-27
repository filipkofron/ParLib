#include "TCPSocket.h"
#include "Common.h"
#include <iostream>

int main(void)
{
  OnStart();
  TCPSocket sock("0.0.0.0", 1234, false);

  OnEnd();
  return 0;
}