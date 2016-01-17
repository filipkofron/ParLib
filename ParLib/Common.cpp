#include <cstring>
#include <cstdarg>
#include <iostream>
#include <sstream>
#include <chrono>

#ifndef _WIN32
#include <unistd.h>
#endif

#include "Common.h"
#include "TCPSocket.h"
#include <fstream>
#include <random>

bool TerminationInProgress = false;

static std::mutex GCommonLock;

int DEBUGVerbose = 0;

std::vector<std::string> SplitIPV4Addr(const std::string& addrStr)
{
  std::vector<std::string> parts;
  size_t len = addrStr.size();
  const char* cstr = addrStr.c_str();
  parts.push_back(std::string());
  for (size_t i = 0; i < len; i++)
  {
    if (cstr[i] == '.')
    {
      parts.push_back(std::string());
    }
    else
    {
      parts[parts.size() - 1] += cstr[i];
    }
  }

  return parts;
}

union addr_ipv4_t
{
  uint32_t address;
  uint8_t bytes[4];
};

uint32_t ParseIPV4Addr(const std::string& addrStr)
{
  addr_ipv4_t addr;
  addr.address = 0;
  std::vector<std::string> parts = SplitIPV4Addr(addrStr);
  if (parts.size() != 4)
    return -1;
  bool littleEndian = static_cast<void*>(&addr.address) == static_cast<void*>(addr.bytes);
  for (int i = 0; i < 4; i++)
  {
    int endianIdx = littleEndian ? 3 - i : i;
    if (parts[i] == "*")
    {
      addr.bytes[endianIdx] = 0;
    }
    else
    {
      std::stringstream ss;
      ss << parts[i];
      int num = 0;
      ss >> num;
      addr.bytes[endianIdx] = num;
    }
  }
  return addr.address;
}

void OnStart()
{
#ifdef _WIN32
  InitSockets();
#endif // _WIN32

  std::cout << "ParLib booting up." << std::endl;
}

void OnEnd(bool waitForUserInput)
{
  TerminationInProgress = true;
#ifdef _WIN32
  DeinitSockets();
#endif // _WIN32

  std::cout << "Press any key to exit." << std::endl;
  std::cin.get();
}

void Info(const char* format, ...)
{
  std::lock_guard<std::mutex> guard(GCommonLock);
  size_t maxLen = strlen(format) + 1024;
  va_list args;
  va_start(args, format);
  char* buffer = new char[maxLen + 1];
  memset(buffer, 0, maxLen + 1);
  vsnprintf(buffer, maxLen, format, args);
  va_end(args);
  buffer[maxLen] = '\0';
  std::cerr << "Info: " << buffer << std::endl;
  delete[] buffer;
}

void Error(const char* format, ...)
{
  std::lock_guard<std::mutex> guard(GCommonLock);
  size_t maxLen = strlen(format) + 1024;
  va_list args;
  va_start(args, format);
  char* buffer = new char[maxLen + 1];
  memset(buffer, 0, maxLen + 1);
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
  OnEnd(true);
  exit(1);
}

static std::shared_ptr<std::ofstream> DebugLog;
static std::mutex DebugLogLock;

void DebugFile(const char* format, ...)
{
  std::lock_guard<std::mutex> lockGuard(DebugLogLock);
  if (!DebugLog)
  {
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> dis(1, 6666666);
    std::string filename = "debug_" + std::to_string(dis(gen)) + ".txt";
    DebugLog = std::make_shared<std::ofstream>(filename.c_str());
  }
  va_list args;
  va_start(args, format);
  size_t maxLen = strlen(format) + 1024;
  char* buffer = new char[maxLen + 1];
  memset(buffer, 0, maxLen + 1);
  vsnprintf(buffer, maxLen, format, args);
  *DebugLog << buffer << std::endl;
  DebugLog->flush();
  va_end(args);
  delete[] buffer;
}

void sleepMs(unsigned long milis)
{
  std::this_thread::sleep_for(std::chrono::milliseconds(milis));
}

int64_t millis()
{
  std::chrono::milliseconds ms = std::chrono::duration_cast<std::chrono::milliseconds>(
    std::chrono::system_clock::now().time_since_epoch()
    );
  return ms.count();
}
