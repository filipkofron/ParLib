#include "TCPSocket.h"
#include "Common.h"
#include <cstring>

#define ZERO_VAR(variable) \
  do { memset(&variable, 0, sizeof(variable)); } while(false)

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
    Error("bind failed with error: %d", WSAGetLastError());
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
#endif

void TCPSocket::Init()
{
  _socket = INVALID_SOCKET;
#ifdef _WIN32
  ZERO_VAR(_acceptedAddr);
  ZERO_VAR(_addrHints);
#endif
  _readFDs = { 0 };
  _tv = { 0 };
  _tv.tv_usec = 500000; // 500ms
  _recvTimeout = 30000; // 30s
  _sendTimeout = 30000; // 30s
  _client = false;
}

TCPSocket::TCPSocket(const std::string& addr, int port, bool client)
  : _client(client)
{
  Init();
#ifdef _WIN32
  if (_client)
  {
    TCPSocketWin32Client(addr, std::to_string(port));
  }
  else
  {
    TCPSocketWin32Server(addr, std::to_string(port));
  }
#endif _WIN32
}

#ifdef _WIN32
TCPSocket::TCPSocket(SOCKET socket, const struct sockaddr& acceptedAddr)
{
  Init();
}
#endif

std::shared_ptr<TCPSocket> TCPSocket::Accept()
{
  struct sockaddr acceptedAddr;
  ZERO_VAR(acceptedAddr);
  int acceptedAddrSize = sizeof(acceptedAddr);
  
  FD_ZERO(&_readFDs);
  FD_SET(_socket, &_readFDs);
  int selectResult = select(0, &_readFDs, nullptr, nullptr, &_tv);
  if (selectResult == SOCKET_ERROR)
  {
    FatalError("Select on server socket failed.");
  }

  if (FD_ISSET(_socket, &_readFDs))
  {
    SOCKET clientSocket = accept(_socket, &acceptedAddr, &acceptedAddrSize);
    if (clientSocket == INVALID_SOCKET)
    {
      Error("accept() returned an invalid socket.");
    }
    else
    {
      setsockopt(clientSocket, SOL_SOCKET, SO_RCVTIMEO, reinterpret_cast<const char *>(&_recvTimeout), sizeof(_recvTimeout));
      setsockopt(clientSocket, SOL_SOCKET, SO_SNDTIMEO, reinterpret_cast<const char *>(&_sendTimeout), sizeof(_sendTimeout));

      return std::make_shared<TCPSocket>(clientSocket, acceptedAddr);
    }
  }
  return nullptr; // select timed out or an accept failure
}

int TCPSocket::Send(char* buffer, int length)
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
      if (shutdown(_socket, SD_SEND) == SOCKET_ERROR)
      {
        Error("Could not shutdown client socket.");
      }
    }
    closesocket(_socket);
  }
}
