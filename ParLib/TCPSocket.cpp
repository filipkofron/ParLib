#include <cstring>
#include <cerrno>
#include <sstream>
#include <iostream>
#include "TCPSocket.h"
#include "Common.h"

#ifdef _WIN32
#include <winsock2.h>
#include <ws2tcpip.h>
#include <iphlpapi.h>

#pragma comment(lib, "iphlpapi.lib")
#pragma comment(lib, "ws2_32.lib")
#else // _WIN32
#include <arpa/inet.h>
#include <sys/socket.h>
#include <netdb.h>
#include <ifaddrs.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <linux/if_link.h>
#endif // _WIN32

#define ZERO_VAR(variable) \
  do { memset(&variable, 0, sizeof(variable)); } while(false)

#ifndef INVALID_SOCKET
#define INVALID_SOCKET -1
#endif

#ifdef _WIN32
WSADATA GwsaData;

void InitSockets()
{
  int result = WSAStartup(MAKEWORD(2, 2), &GwsaData);
  if (result != 0)
  {
    FatalError("WSAStartup failed with error: %d", result);
  }
}

void DeinitSockets()
{
  WSACleanup();
}
#endif

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

static uint32_t ParseIPV4Addr(const std::string& addrStr)
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

#ifdef _WIN32
void TCPSocket::TCPSocketWin32Server(const std::string& addr, const std::string& port)
{
  _socket = INVALID_SOCKET;
  struct addrinfo* resultAddr;
  
  _addrHints.ai_family = AF_INET;
  _addrHints.ai_socktype = SOCK_STREAM;
  _addrHints.ai_protocol = IPPROTO_TCP;
  _addrHints.ai_flags = AI_PASSIVE;

  int result = getaddrinfo(addr.c_str(), port.c_str(), &_addrHints, &resultAddr);
  if (result != 0)
  {
    Error("getaddrinfo failed with error: %d", result);
    return;
  }

  _socket = socket(resultAddr->ai_family, resultAddr->ai_socktype, resultAddr->ai_protocol);
  if (_socket == INVALID_SOCKET)
  {
    freeaddrinfo(resultAddr);
    Error("socket failed with error: %ld", WSAGetLastError());
    return;
  }

  // Setup the TCP listening socket
  result = bind(_socket, resultAddr->ai_addr, static_cast<int>(resultAddr->ai_addrlen));
  if (result == SOCKET_ERROR) {
    freeaddrinfo(resultAddr);
    closesocket(_socket);
    _socket = INVALID_SOCKET;
    Error("bind failed with error: %d", WSAGetLa#ifdef _WIN32stError());
    return;
  }

  freeaddrinfo(resultAddr);

  result = listen(_socket, SOMAXCONN);
  if (result == SOCKET_ERROR) {
    closesocket(_socket);
    _socket = INVALID_SOCKET;
    Error("listen failed with error: %d", WSAGetLastError());
  }
}

void TCPSocket::TCPSocketWin32Client(const std::string addr, const std::string& port)
{
  struct addrinfo* resultAddr;
  struct addrinfo* ptr;

  _addrHints.ai_family = AF_UNSPEC;
  _addrHints.ai_socktype = SOCK_STREAM;
  _addrHints.ai_protocol = IPPROTO_TCP;

  // Resolve the server address and port
  int result = getaddrinfo(addr.c_str(), port.c_str(), &_addrHints, &resultAddr);
  if (result != 0)
  {
    Error("getaddrinfo failed with error: %d\n", result);
    return;
  }

  for (ptr = resultAddr; ptr != nullptr; ptr = ptr->ai_next) {

  // Create a SOCKET for connecting to server
  _socket = socket(ptr->ai_family, ptr->ai_socktype, ptr->ai_protocol);
  if (_socket == INVALID_SOCKET)
  {
    Error("socket failed with error: %d", WSAGetLastError());
  }

  // Connect to server.
  result = connect(_socket, ptr->ai_addr, static_cast<int>(ptr->ai_addrlen));
  if (result == SOCKET_ERROR)
  {
    closesocket(_socket);
    _socket = INVALID_SOCKET;
    continue;
  }
  break;
}

freeaddrinfo(resultAddr);

if (_socket == INVALID_SOCKET)
{
  Error("Unable to connect to server!\n");
  return;
}
}

void TCPSocket::TCPSocketWin32AcceptedClient(SOCKET socket, const struct sockaddr& acceptedAddr)
{
  _client = true;
  _socket = socket;
  _acceptedAddr = acceptedAddr;
}
#else // _WIN32
static int HostnameToIp(const char* hostname, char* ip)
{
  struct hostent* he;
  struct in_addr** addr_list;
  int i;
	
  if ((he = gethostbyname(hostname)) == NULL) 
  {
    // get the host info
    Error("gethostbyname");
    return 1;
  }

  addr_list = (struct in_addr **) he->h_addr_list;

  for(i = 0; addr_list[i] != NULL; i++) 
  {
    //Return the first one;HostnameToIp
    strcpy(ip, inet_ntoa(*addr_list[i]));
    return 0;
  }

  return 1;
}

void TCPSocket::TCPSocketLinuxServer(const std::string& addr, int port)
{
  _socket = socket(AF_INET, SOCK_STREAM, 0);
  if (_socket < 0)
  {
    _socket = INVALID_SOCKET;
    FatalError("Cannot create listen socket!");
  }

  int rc = 0, on = 1;
  sockaddr_in listenAddr;

  rc = setsockopt(_socket, SOL_SOCKET,  SO_REUSEADDR, (char *) &on, sizeof(on));
  if (rc < 0)
  {
    close(_socket);
    _socket = INVALID_SOCKET;
    FatalError("setsockopt() failed!");
  }

  memset(&listenAddr, 0, sizeof(listenAddr));
  listenAddr.sin_family = AF_INET;
  char ip[100];
  HostnameToIp(addr.c_str(), ip);
  unsigned int addrHost = ParseIPV4Addr(ip);
  std::cout << "addrHost: " << addrHost << std::endl;
  listenAddr.sin_addr.s_addr = htonl(addrHost);
  listenAddr.sin_port = htons(port);
  rc = bind(_socket, (struct sockaddr *)&listenAddr, sizeof(listenAddr));
  if (rc < 0)
  {
    close(_socket);
    _socket = INVALID_SOCKET;
    FatalError("bind() failed!");
  }

  rc = setsockopt (_socket, SOL_SOCKET, SO_RCVTIMEO, (char *) &_tv, sizeof(_tv));
  if(rc < 0)
  {
    close(_socket);
    _socket = INVALID_SOCKET;
    FatalError("setsockopt() failed!");
  }

  rc = listen(_socket, 16);
  if (rc < 0)
  {
    close(_socket);
    _socket = INVALID_SOCKET;
    FatalError("listen() failed!");
  }
}

void TCPSocket::TCPSocketLinuxClient(const std::string addr, int port)
{
  _socket = socket(AF_INET, SOCK_STREAM, 0);
  if (_socket < 0)
  {
    _socket = INVALID_SOCKET;
    FatalError("Cannot create socket!");
  }

  int rc = 0, on = 1;
  sockaddr_in connAddr;

  rc = setsockopt(_socket, SOL_SOCKET,  SO_REUSEADDR, (char *) &on, sizeof(on));
  if (rc < 0)
  {
    close(_socket);
    _socket = INVALID_SOCKET;
    Error("setsockopt() failed!");
    return;
  }

  memset(&connAddr, 0, sizeof(connAddr));
  connAddr.sin_family = AF_INET;
  char ip[100];
  HostnameToIp(addr.c_str(), ip);
  std::cout << "hostname: " << addr.c_str() << " ip: " << ip << " port: " << port<<  std::endl;
  connAddr.sin_addr.s_addr = htonl(ParseIPV4Addr(ip));
  connAddr.sin_port = htons(port);
  printf("connAddr.sin_addr.s_addr: %X", connAddr.sin_addr.s_addr);
  rc = connect(_socket, (struct sockaddr *) &connAddr , sizeof(connAddr));
  if (rc < 0)
  {
    close(_socket);
    _socket = INVALID_SOCKET;
    Error("connect failed. Error: %s", strerror(errno));
    return;
  }
}

void TCPSocket::TCPSocketLinuxAcceptedClient(int socket, const struct sockaddr_in& acceptedAddr)
{
  _client = true;
  _socket = socket;
  _acceptedAddr = acceptedAddr;
}
#endif // _WIN32

void TCPSocket::Init()
{
  ZERO_VAR(_acceptedAddr);
#ifdef _WIN32
  ZERO_VAR(_addrHints);
  _readFDs = { 0 };
#else // _WIn32
#endif // _WIn32
  _socket = INVALID_SOCKET;
  _tv = { 0 };
  _tv.tv_usec = 500000; // 500ms
  _recvTimeout = 30000; // 30s
  _sendTimeout = 30000; // 30s
#ifndef _WIN32
  _acceptTimeout = 500; // 500ms
#endif // !_WIN32
  _client = false;
}

TCPSocket::TCPSocket(const std::string& addr, int port, bool client)
{
  Init();
  _client = client;

  if (_client)
  {
#ifdef _WIN32
    TCPSocketWin32Client(addr, std::to_string(port));
#else // _WIN32
    TCPSocketLinuxClient(addr, port);
#endif // _WIN32
  }
  else
  {
#ifdef _WIN32
    TCPSocketWin32Server(addr, std::to_string(port));
#else // _WIN32
	TCPSocketLinuxServer(addr, port);
#endif // _WIN32
  }
}

#ifdef _WIN32
TCPSocket::TCPSocket(SOCKET socket, const struct sockaddr& acceptedAddr)
#else // _WIN32
TCPSocket::TCPSocket(int socket, const struct sockaddr_in& acceptedAddr)
#endif // _WIN32
{
  Init();
}

std::shared_ptr<TCPSocket> TCPSocket::Accept()
{
#ifdef _WIN32
  struct sockaddr acceptedAddr;
  int acceptedAddrSize = sizeof(acceptedAddr);
#else // _WIN32
  struct sockaddr_in acceptedAddr;
  socklen_t acceptedAddrSize = (socklen_t) sizeof(sockaddr_in);
#endif // _WIN32
  ZERO_VAR(acceptedAddr);
#ifdef _WIN32
  FD_ZERO(&_readFDs);
  FD_SET(_socket, &_readFDs);
  int selectResult = select(0, &_readFDs, nullptr, nullptr, &_tv);
  if (selectResult == SOCKET_ERROR)
  {
    if (!TerminationInProgress)
    {
      FatalError("Select on server socket failed, WSAGetLastError: %i.", WSAGetLastError());
    }
    return nullptr;
  }

  if (FD_ISSET(_socket, &_readFDs))
  {
    SOCKET clientSocket = accept(_socket, &acceptedAddr, &acceptedAddrSize);
    if (clientSocket == INVALID_SOCKET)
    {
      if (!TerminationInProgress)
      {
        Error("accept() returned an invalid socket.");
      }
    }
    else
    {
      setsockopt(clientSocket, SOL_SOCKET, SO_RCVTIMEO, reinterpret_cast<const char *>(&_recvTimeout), sizeof(_recvTimeout));
      setsockopt(clientSocket, SOL_SOCKET, SO_SNDTIMEO, reinterpret_cast<const char *>(&_sendTimeout), sizeof(_sendTimeout));

      return std::make_shared<TCPSocket>(clientSocket, acceptedAddr);
    }
  }
#else // _WIN32
  int clientSocket = accept(_socket, (sockaddr *) &acceptedAddr, &acceptedAddrSize);
  if (clientSocket >= 0)
  {
          setsockopt(clientSocket, SOL_SOCKET, SO_RCVTIMEO, reinterpret_cast<const char *>(&_recvTimeout), sizeof(_recvTimeout));
      setsockopt(clientSocket, SOL_SOCKET, SO_SNDTIMEO, reinterpret_cast<const char *>(&_sendTimeout), sizeof(_sendTimeout));
    return std::make_shared<TCPSocket>(clientSocket, acceptedAddr);
  }
#endif // _WIN32
  return nullptr; // select timed out or an accept failure
}

int TCPSocket::Send(const char* buffer, int length)
{
  return send(_socket, buffer, length, 0);
}

int TCPSocket::Receive(char* buffer, int length)
{
  return recv(_socket, buffer, length, 0);
}

bool TCPSocket::IsOk() const
{
  return _socket != INVALID_SOCKET;
}

TCPSocket::~TCPSocket()
{
  if (_socket != INVALID_SOCKET)
  {
    if (_client)
    {
#ifdef _WIN32
      if (shutdown(_socket, SD_SEND) == SOCKET_ERROR)
      {
        Error("Could not shutdown client socket.");
      }
#endif // _WIN32
    }
#ifdef _WIN32
    closesocket(_socket);
#else // _WIN32
    close(_socket);
#endif // _WIN32
  }
}

std::vector<std::string> TCPSocket::GetLocalAddresses()
{
  std::vector<std::string> addresses;
#ifdef _WIN32
  /* Variables used by GetIpAddrTable */
  PMIB_IPADDRTABLE pIPAddrTable;
  DWORD dwSize = 0;
  DWORD dwRetVal = 0;
  IN_ADDR IPAddr;

  /* Variables used to return error message */
  LPVOID lpMsgBuf;

  // Before calling AddIPAddress we use GetIpAddrTable to get
  // an adapter to which we can add the IP.
  pIPAddrTable = static_cast<MIB_IPADDRTABLE *>(malloc(sizeof(MIB_IPADDRTABLE)));

  if (pIPAddrTable) {
    // Make an initial call to GetIpAddrTable to get the
    // necessary size into the dwSize variable
    if (GetIpAddrTable(pIPAddrTable, &dwSize, 0) ==
      ERROR_INSUFFICIENT_BUFFER) {
      free(pIPAddrTable);
      pIPAddrTable = static_cast<MIB_IPADDRTABLE *>(malloc(dwSize));

    }
    if (pIPAddrTable == nullptr) {
      printf("Memory allocation failed for GetIpAddrTable\n");
      exit(1);
    }
  }
  // Make a second call to GetIpAddrTable to get the
  // actual data we want
  if ((dwRetVal = GetIpAddrTable(pIPAddrTable, &dwSize, 0)) != NO_ERROR) {
    printf("GetIpAddrTable failed with error %d\n", dwRetVal);
    if (FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS, NULL, dwRetVal, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),       // Default language
      (LPTSTR)& lpMsgBuf, 0, NULL)) {
      printf("\tError: %s", static_cast<char const*>(lpMsgBuf));
      LocalFree(lpMsgBuf);
    }
    exit(1);
  }

  const int bufLen = 32;
  char buffer[bufLen];
  for (auto i = 0; i < static_cast<int>(pIPAddrTable->dwNumEntries); i++)
  {
    IPAddr.S_un.S_addr = static_cast<u_long>(pIPAddrTable->table[i].dwAddr);
    inet_ntop(AF_INET, &IPAddr, buffer, bufLen);
    addresses.push_back(buffer);
  }

  if (pIPAddrTable) {
    free(pIPAddrTable);
    pIPAddrTable = nullptr;
  }
#else // _WIN32
  struct ifaddrs *ifaddr, *ifa;
  int family, s, n;
  char host[NI_MAXHOST];

  if (getifaddrs(&ifaddr) == -1)
  {
    FatalError("getifaddrs");
  }

  for (ifa = ifaddr, n = 0; ifa != NULL; ifa = ifa->ifa_next, n++)
  {
    if (ifa->ifa_addr == NULL)
      continue;

    family = ifa->ifa_addr->sa_family;

    printf("%-8s %s (%d)\n",
            ifa->ifa_name,
            (family == AF_PACKET) ? "AF_PACKET" :
            (family == AF_INET) ? "AF_INET" :
            (family == AF_INET6) ? "AF_INET6" : "???",
            family);

    if (family == AF_INET || family == AF_INET6)
    {
      s = getnameinfo(ifa->ifa_addr,
      (family == AF_INET) ? sizeof(struct sockaddr_in) :
                            sizeof(struct sockaddr_in6),
      host, NI_MAXHOST,
      NULL, 0, NI_NUMERICHOST);
      if (s != 0)
      {
        FatalError("getnameinfo() failed: %s\n", gai_strerror(s));
      }

      printf("\t\taddress: <%s>\n", host);
      addresses.push_back(std::string(host));

    }
    else if (family == AF_PACKET && ifa->ifa_data != NULL)
    {
      struct rtnl_link_stats *stats = static_cast<struct rtnl_link_stats *>(ifa->ifa_data);

      printf("\t\ttx_packets = %10u; rx_packets = %10u\n"
             "\t\ttx_bytes   = %10u; rx_bytes   = %10u\n",
             stats->tx_packets, stats->rx_packets,
             stats->tx_bytes, stats->rx_bytes);
    }
  }

   freeifaddrs(ifaddr);
#endif // _WIN32

  return addresses;
}

std::string TCPSocket::GetLocalAddressInSubnet(const std::string& address, int bits)
{
  std::vector<std::string> addresses = GetLocalAddresses();
  uint32_t subnet = ParseIPV4Addr(address);
  subnet = subnet >> bits;
  subnet = subnet << bits;
  for (const auto& addr : addresses)
  {
    uint32_t prefix = ParseIPV4Addr(addr);
    prefix = prefix >> bits;
    prefix = prefix << bits;
    if (prefix == subnet)
    {
      return addr;
    }
  }
  return "";

}
