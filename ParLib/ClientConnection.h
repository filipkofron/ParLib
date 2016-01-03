#pragma once
#include "TCPSocket.h"
#include <thread>

class ClientConnection
{
private:
  std::shared_ptr<TCPSocket> _socket;
  std::shared_ptr<std::thread> _receiverThread;
  std::string _networkId;
  static void ReceiverLoop(std::shared_ptr<ClientConnection> conn, bool client);
  static bool HandshakeRecv(const std::shared_ptr<ClientConnection>& conn);
  static bool HandshakeSend(const std::shared_ptr<ClientConnection>& conn);
  static bool Handshake(const std::shared_ptr<ClientConnection>& conn, bool client);
public:
  ClientConnection(const std::shared_ptr<TCPSocket>& socket);
  ~ClientConnection();
  void CleanUp();
  void SendKeepAlive();
  void SendKeepAliveResp();
  static void StartReceiverThread(std::shared_ptr<ClientConnection> conn, bool client);
  const std::string& GetNetworkId();
};