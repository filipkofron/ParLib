#include <cstring>
#include <cstdarg>
#include <iostream>

#include "Common.h"
#include "TCPSocket.h"

bool TerminationInProgress = false;

void OnStart()
{
  InitSockets();

  std::cout << "ParLib booting up." << std::endl;
}

void OnEnd()
{
  TerminationInProgress = true;
  DeinitSockets();

  std::cout << "Press any key to exit." << std::endl;
  std::cin.get();
}

void Info(const char* format, ...)
{
  size_t maxLen = strlen(format) + 1024;
  va_list args;
  va_start(args, format);
  char* buffer = new char[maxLen + 1];
  vsnprintf(buffer, maxLen, format, args);
  va_end(args);
  buffer[maxLen] = '\0';
  std::cerr << "Info: " << buffer << std::endl;
  delete[] buffer;
}

void Error(const char* format, ...)
{
  size_t maxLen = strlen(format) + 1024;
  va_list args;
  va_start(args, format);
  char* buffer = new char[maxLen + 1];
  vsnprintf(buffer, maxLen, format, args);
  va_end(args);
  buffer[maxLen] = '\0';
  std::cerr << "Error: " << buffer << std::endl;
  delete[] buffer;
}

void FatalError(const char* format, ...)
{
  va_list args;
  va_start(args, format);
  Error(format, args);
  va_end(args);
  OnEnd();
  exit(1);
}
