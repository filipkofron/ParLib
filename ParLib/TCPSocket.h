#pragma once

#include <string>
#include <memory>
#include <vector>
#include <memory>

#ifdef _WIN32
#undef UNICODE

#define WIN32_LEAN_AND_MEAN

#include <windows.h>
#include <winsock2.h>
#include <ws2tcpip.h>

#pragma comment (lib, "Ws2_32.lib")
#else // _WIN32
#include <netinet/in.h>
#include <sys/ioctl.h>
#endif // _WIN32


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
#else // _WIN32
  int _socket;
  timeval _tv;
  unsigned int _recvTimeout;
  unsigned int _sendTimeout;
  unsigned int _acceptTimeout;
  sockaddr_in _acceptedAddr;
#endif // _WIN32
  bool _client;
  /*!
   * Initialize structures, must be called from each constructor upon start.
   */
  void Init();

  TCPSocket(const TCPSocket& src) { }
#ifdef _WIN32
  void TCPSocketWin32Server(const std::string& addr, const std::string& port);
  void TCPSocketWin32Client(const std::string addr, const std::string& port);
  void TCPSocketWin32AcceptedClient(SOCKET socket, const struct sockaddr& acceptedAddr);
#else // _WIN32
  void TCPSocketLinuxServer(const std::string& addr, int port);
  void TCPSocketLinuxClient(const std::string addr, int port);
  void TCPSocketLinuxAcceptedClient(int socket, const struct sockaddr_in& acceptedAddr);
#endif // _WIN32
public:
  TCPSocket(const std::string& addr, int port, bool client, int timeout = 500);
#ifdef _WIN32
  TCPSocket(SOCKET socket, const struct sockaddr& acceptedAddr);
#else // _WIN32
  TCPSocket(int socket, const struct sockaddr_in& acceptedAddr);
#endif // _WIN32

  std::shared_ptr<TCPSocket> Accept();

  int Send(const char* buffer, int length);
  int Receive(char* buffer, int length);

  bool IsOk() const;

  void ResetTimeouts();
  void SetTimeout(uint32_t timeOut);

  ~TCPSocket();

  static std::vector<std::string> GetLocalAddresses();
  static std::string GetLocalAddressInSubnet(const std::string& address, int bits);
};
