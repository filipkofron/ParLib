#include "TCPSocket.h"
#include "ClientConnection.h"
#include "MessageFactory.h"
#include <iostream>

void ClientConnection::ReceiverLoop(std::shared_ptr<ClientConnection> conn, bool client)
{
  conn->_socket->SetTimeout(30000);
  std::cout << "Handshake started." << std::endl;
  if (!Handshake(conn, client))
  {
    // TODO: Disconnect client.
    GNetworkManager->RegisterFinishingClient(conn);
    Error("Handshake failed!");
    return;
  }
  conn->_lastTimeAlive = millis();
  conn->_socket->ResetTimeouts();
  conn->_socket->SetTimeout(CLIENT_CONNECTION_TIMEOUT_MS);
  if (!GNetworkManager->AddOrDiscardClient(conn, client))
  {
    std::cout << "NOT connected with <" << conn->GetNetworkId() << "> client: " << client << std::endl;
    GNetworkManager->RegisterFinishingClient(conn);
    return;
  }
  std::cout << "Connected with <" << conn->GetNetworkId() << "> client: " << client << std::endl;
   
  while (conn->_socket->IsOk())
  {
    auto msg = Message::Receive(*conn->_socket);
    if (msg)
    {
      conn->_lastTimeAlive = millis();
      GNetworkManager->OnMessage(std::make_shared<ReceivedMessage>(conn->GetNetworkId(), msg));
    }
    if (millis() - conn->_lastTimeAlive > CLIENT_CONNECTION_TIMEOUT_MS)
    {
      std::cout << "Socket timeout for " << conn->GetNetworkId() << " client: " << client << std::endl;
      break;
    }
  }
  std::cout << "Socket failure with " << conn->GetNetworkId() << " client: " << client << std::endl;
  // TODO: Disconnect client.
  GNetworkManager->RegisterFinishingClient(conn);
}

bool ClientConnection::HandshakeRecv(const std::shared_ptr<ClientConnection>& conn)
{
  auto foreignInitial = Message::Receive(*conn->_socket);
  if (!conn->_socket->IsOk() || !foreignInitial || foreignInitial->GetType() != MESSAGE_TYPE_INITIAL)
  {
    return false;
  }
  conn->_networkId = "";
  auto len = foreignInitial->GetLength();
  const char* data = foreignInitial->GetData();
  for (auto i = 0; i < len; i++)
  {
    conn->_networkId += data[i];
  }
  return true;
}

bool ClientConnection::HandshakeSend(const std::shared_ptr<ClientConnection>& conn)
{
  auto initialMsg = MessageFactory::CreateInitialMessage(GNetworkManager->GetNetworkId());
  if (!conn->_socket->IsOk() || !initialMsg->Send(*conn->_socket))
  {
    return false;
  }
  return true;
}

bool ClientConnection::Handshake(const std::shared_ptr<ClientConnection>& conn, bool client)
{
  if (client)
  {
    bool res = HandshakeSend(conn);
    if (!res)
    {
      return false;
    }
    return HandshakeRecv(conn);
  }
  else
  {
    bool res = HandshakeRecv(conn);
    if (!res)
    {
      return false;
    }
    return HandshakeSend(conn);
  }
}

ClientConnection::ClientConnection(const std::shared_ptr<TCPSocket>& socket)
  : _socket(socket)
{

}

void ClientConnection::CleanUp()
{
  if (_socket)
  {
    _socket->Close();
  }  
  if (_receiverThread)
  {
    _receiverThread->join();
    _receiverThread = nullptr;
  }
  if (_networkId.size() > 0)
  {
    GNetworkManager->DiscardClient(this);
  }
  _socket = nullptr;
}

void ClientConnection::SendKeepAlive()
{
  MessageFactory::CreateKeepAliveMessage()->Send(*_socket);
}

void ClientConnection::SendKeepAliveResp()
{
  MessageFactory::CreateKeepAliveMessageResp()->Send(*_socket);
}

bool ClientConnection::SendMessage(const Message& msg)
{
  return msg.Send(*_socket);
}

ClientConnection::~ClientConnection()
{
  CleanUp();
}

void ClientConnection::StartReceiverThread(std::shared_ptr<ClientConnection> conn, bool client)
{
  if (!conn->_receiverThread)
  {
    std::thread* thr = new std::thread(&ClientConnection::ReceiverLoop, conn, client);
   conn->_receiverThread = std::shared_ptr<std::thread>(thr);
  }
  else
  {
    GNetworkManager->RegisterFinishingClient(conn);
  }
}

const std::string& ClientConnection::GetNetworkId()
{
  if (!_networkId.size())
  {
    FatalError("Client with an unknown ID.");
  }

  return _networkId;
}
