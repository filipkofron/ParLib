#pragma once

#include <string>

#ifdef _WIN32
#undef UNICODE

#define WIN32_LEAN_AND_MEAN

#include <windows.h>
#include <winsock2.h>
#include <ws2tcpip.h>

#pragma comment (lib, "Ws2_32.lib")
#endif
#include <memory>

#ifdef _WIN32
extern WSADATA GwsaData;
void InitSockets();
void DeinitSockets();
#endif

/*!
 * Wrapping around an OS specific TCP socket.
 */
class TCPSocket
{
private:
#ifdef _WIN32
  SOCKET _socket;
  struct sockaddr _acceptedAddr;
  struct addrinfo _addrHints;
  TIMEVAL _tv;
  DWORD _recvTimeout;
  DWORD _sendTimeout;
  fd_set _readFDs;
#endif
  /*!
   * Initialize structures, must be called from each constructor upon start.
   */
  void Init();

  TCPSocket(const TCPSocket& src) { }
#ifdef _WIN32
  void TCPSocketWin32Server(const std::string& addr, const std::string& port);
  void TCPSocketWin32AcceptedClient(SOCKET socket, const struct sockaddr& acceptedAddr);
#endif
public:
  TCPSocket(const std::string& addr, int port);
#ifdef _WIN32
  TCPSocket(SOCKET socket, const struct sockaddr& acceptedAddr);
#endif

  std::shared_ptr<TCPSocket> Accept();

  int Send(char* buffer, int length);
  int Receive(char* buffer, int length);

  ~TCPSocket();
};