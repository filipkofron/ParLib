#include "TCPSocket.h"
#include "ClientConnection.h"
#include "MessageFactory.h"

void ClientConnection::ReceiverLoop()
{
  auto msg = MessageFactory::CreateInitialMessage(GNetworkManager->GetNetworkId());
  msg->Send(*_socket);
}

ClientConnection::ClientConnection(const std::shared_ptr<TCPSocket>& socket)
  : _socket(socket)
{

}

void ClientConnection::StartReceiverThread()
{
  if (!_receiverThread)
  {
    std::thread* thr = new std::thread(&ClientConnection::ReceiverLoop, this);
    _receiverThread = std::shared_ptr<std::thread>(thr);
  }
}
