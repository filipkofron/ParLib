#include "NetworkManager.h"
#include "Common.h"
#include "AddrIterator.h"
#include <iostream>
#include "MessageFactory.h"

#define ELECTION_TIME_OUT 500

void NetworkManager::AddFoundServers(const std::vector<std::shared_ptr<TCPSocket> >& sockets)
{
  for (auto& socket : sockets)
  {
    auto client = std::make_shared<ClientConnection>(socket);
    client->StartReceiverThread(client, true);
  }
}

NetworkManager::NetworkManager(const std::string& network, int maskBits)
  : _network(network), _maskBits(maskBits),
  _terminate(false),
  _discovered(false),
  _lostLeader(false),
  _started(millis()),
  _keepAliveThread(&NetworkManager::KeepAliveLoop, this),
  _mainLoopThread(&NetworkManager::MainLoop, this),
  _leader(false),
  _sentElectionTime(millis()),
  _electionParticipant(false)
{
  std::string localAddr = TCPSocket::GetLocalAddressInSubnet(network, maskBits);
  _serverConnection = std::make_shared<ServerConnection>();
  if (!_serverConnection->InitServer())
  {
    FatalError("Cannot initialize the NetworkManager's server.");
  }
  _networkId = localAddr + "_" + std::to_string(_serverConnection->GetPort());
  if (!_serverConnection->StartServer())
  {
    FatalError("Cannot start accepting clients on the NetworkManager's server.");
  }
}

std::shared_ptr<ClientConnection> NetworkManager::FindClient(const std::string& networkId)
{
  std::lock_guard<std::mutex> guard(_lock);
  std::shared_ptr<ClientConnection> conn(nullptr);
  if (_clientConnections.find(networkId) != _clientConnections.end())
  {
    conn = _clientConnections[networkId];
  }
  return conn;
}

void NetworkManager::CleanFinishingClients()
{
  std::lock_guard<std::mutex> guard(_finishingLock);
  for (auto client : _finishingClients)
  {
    client->CleanUp();
  }
  _finishingClients.clear();
}

void NetworkManager::OnKeepAliveMessage(const std::shared_ptr<ReceivedMessage>& msg)
{
  auto conn = FindClient(msg->GetSenderId());

  if (!conn)
    return;

  conn->SendKeepAliveResp();
}

void NetworkManager::OnElectionMessage(const std::shared_ptr<ReceivedMessage>& msg)
{
  std::string electId = msg->GetMsg()->AsString();
  std::cout << "Election message came from " << electId << "!" << std::endl;

  if (electId > GetNetworkId())
  {
    SendMsg(*msg->GetMsg(), GetNextId(GetNetworkId()));
  }
  else if (electId < GetNetworkId() && !_electionParticipant)
  {
    auto msg = MessageFactory::CreateElectionMessage(GetNetworkId());
    SendMsg(*msg, GetNextId(GetNetworkId()));
  }
  else if (electId == GetNetworkId())
  {
    _leaderId = GetNetworkId();
    _leader = true;
    std::cout << "Me " << _leaderId << " is leader!" << std::endl;
    _electionParticipant = false;
    auto msg = MessageFactory::CreateElectedMessage(GetNetworkId());
    SendMsg(*msg, GetNextId(GetNetworkId()));
  }
  // TODO
}

void NetworkManager::OnElectedMessage(const std::shared_ptr<ReceivedMessage>& msg)
{
  std::string electId = msg->GetMsg()->AsString();
  std::cout << "Elected message came with " << electId << " from " << msg->GetSenderId() << std::endl;

  if (electId == GetNetworkId())
  {
    std::cout << "Elected message came back to the origin " << electId << std::endl;
    return;
  }

  _electionParticipant = false;
  _leaderId = electId;
  _leader = false;
}

void NetworkManager::OnKnownLeaderMessage(const std::shared_ptr<ReceivedMessage>& msg)
{
  std::string electId = msg->GetMsg()->AsString();
  std::cout << "Known leader message came with " << electId << " from " << msg->GetSenderId() << std::endl;

  if (electId == GetNetworkId())
    return;

  _electionParticipant = false;
  _leaderId = electId;
  _leader = false;
}

void NetworkManager::MainLoop()
{
  while (!_terminate)
  {
    Step();
  }
}

void NetworkManager::Step()
{
  int64_t currTime = millis();
  if (_discovered && _leaderId.empty() && (currTime - _sentElectionTime) > ELECTION_TIME_OUT)
  {
    auto msg = MessageFactory::CreateElectionMessage(GetNetworkId());
    std::string destId = GetNextId(GetNetworkId());
    if (destId.empty())
    {
      if (currTime - _started > 10000)
      {
        std::cout << "No other host came up in 10 seconds, selecting myself as the leader." << std::endl;
        _leader = true;
        _leaderId = GetNetworkId();
      }
    }
    else
    {
      SendMsg(*msg, destId);
      _sentElectionTime = currTime;
    }
  }
  if (_lostLeader)
  {
    auto leader = FindClient(_leaderId);
    if (!leader)
    {
      _leader = false;
      _lostLeader = false;
      _leaderId.clear();
    }
  }
  sleepMs(100);
}

bool NetworkManager::CheckForServer(TCPSocket* socket)
{
  char b = 0;
  socket->SetTimeout(5);
  int rec = socket->Receive(&b, 1);
  return socket->IsOk() && rec == 1 && b == 66;
}

void NetworkManager::DiscoverAll()
{
  sleepMs(1000);
  _discovered = false;
  std::cout << "Discovering all nodes on the network " << _network << ". " << (1 << _maskBits) << " hosts will be scanned." << std::endl;
  int defaultPort = DEFAULT_PORT;
  int maxPort = defaultPort + 8;
  AddrSubnetIterator addrIterator(_network, _maskBits);
  std::vector<std::shared_ptr<TCPSocket> > foundSockets;
#ifndef _WIN32
  const int maxTries = 16;
#endif // _WIN32
  int tries = 0;
  while (true)
  {
#ifndef _WIN32
    if (tries > maxTries)
    {
      break;
    }
#endif // _WIN32
    tries++;
    std::string next = addrIterator.NextAddr();
    if (!next.size())
    {
      break;
    }
    for (int i = defaultPort; i < maxPort; i++)
    {
      // std::cout << "Trying " << next << ":" << i << std::endl;
      TCPSocket* socket = new TCPSocket(next, i, true, 5);
      if (CheckForServer(socket))
      {
        socket->ResetTimeouts();
        foundSockets.push_back(std::shared_ptr<TCPSocket> (socket));
      }
      else
      {
        delete socket;
      }
    }
  }
  AddFoundServers(foundSockets);
  _discovered = true;
}

void NetworkManager::KeepAliveLoop()
{
  while (!_terminate)
  {
    sleepMs(100);
    CleanFinishingClients();
   // std::cout << "Sending keep alive messages!" << std::endl;
    std::lock_guard<std::mutex> guard(_lock);
    for (auto& client : _clientConnections)
    {
      if (DEBUGVerbose) std::cout << "Sending keep alive message to: " << client.first << std::endl;
      client.second->SendKeepAlive();
    }
  }
}

void NetworkManager::DisconnectClients()
{
  CleanFinishingClients();
  std::lock_guard<std::mutex> guard(_lock);
  _clientConnections.clear();
}

void NetworkManager::Terminate()
{
  _terminate = true;
  CleanFinishingClients();
  _serverConnection->StopServer();

  _keepAliveThread.join();
  _mainLoopThread.join();
  std::lock_guard<std::mutex> guard(_lock);
  _clientConnections.clear();
}

void NetworkManager::RegisterFinishingClient(std::shared_ptr<ClientConnection> client)
{
  CleanFinishingClients();
  std::lock_guard<std::mutex> guard(_finishingLock);
  _finishingClients.push_back(client);
}

void NetworkManager::OnMessage(const std::shared_ptr<ReceivedMessage>& msg)
{
  CleanFinishingClients();
  if (DEBUGVerbose) std::cout << "Received message type: " << msg->GetMsg()->GetType() << std::endl;
  switch (msg->GetMsg()->GetType())
  {
  case MESSAGE_TYPE_KEEP_ALIVE:
    OnKeepAliveMessage(msg);
    break;
  case MESSAGE_TYPE_ELECTION:
    OnElectionMessage(msg);
    break;
  case MESSAGE_TYPE_ELECTED:
    OnElectedMessage(msg);
    break;
  case MESSAGE_TYPE_KNOWN_LEADER:
    OnKnownLeaderMessage(msg);
    break;
  case MESSAGE_TYPE_STACK_ASSIGNMENT:
    GComputation->AddMessage(msg);
    break;
  case MESSAGE_TYPE_ASSIGNMENT_FINISHED:
    GComputation->AddMessage(msg);
    break;
  case MESSAGE_TYPE_REQUEST_RETURN_STACK:
    GComputation->AddMessage(msg);
    break;
  case MESSAGE_TYPE_RETURNING_STACK:
    GComputation->AddMessage(msg);
    break;
  case MESSAGE_TYPE_TERMINATE:
    GComputation->AddMessage(msg);
    break;
  }
}

bool NetworkManager::SendMsg(const Message& msg, const std::string& destId)
{
  std::shared_ptr<ClientConnection> client = FindClient(destId);
  if (client)
  {
    return client->SendMsg(msg);
  }

  return false;
}

bool NetworkManager::BroadcastMsg(const Message& msg)
{
  std::lock_guard<std::mutex> guard(_lock);
  for (auto& client : _clientConnections)
  {
    return client.second->SendMsg(msg);
  }

  return false;
}

bool NetworkManager::AddOrDiscardClient(const std::shared_ptr<ClientConnection>& client, bool isClient)
{
  if (client->GetNetworkId() == GetNetworkId())
    return false;

  std::lock_guard<std::mutex> guard(_lock);
  if (_clientConnections.find(client->GetNetworkId()) == _clientConnections.end())
  {
    _clientConnections[client->GetNetworkId()] = client;
    return true;
  }
  if ((client->GetNetworkId() < GetNetworkId()) == isClient)
  {
    _clientConnections[client->GetNetworkId()] = client;
    return true;
  }
  return false;
}

void NetworkManager::DiscardClient(ClientConnection* client)
{
  std::lock_guard<std::mutex> guard(_lock);
  if (_clientConnections.find(client->GetNetworkId()) == _clientConnections.end())
    return;
  if (_clientConnections[client->GetNetworkId()].get() == client)
    _clientConnections.erase(client->GetNetworkId());
  if (client->GetNetworkId() == _leaderId)
    _lostLeader = true;
}

std::vector<std::string> NetworkManager::GetClients()
{
  std::vector<std::string> res;
  std::lock_guard<std::mutex> guard(_lock);
  for (auto& client : _clientConnections)
  {
    res.push_back(client.first);
  }
  return res;
}

const std::string& NetworkManager::GetNetworkId() const
{
  if (!_networkId.size())
  {
    FatalError("Network id unknown!");
  }
  return _networkId;
}

std::string NetworkManager::GetNextId(const std::string& prev) const
{
  std::lock_guard<std::mutex> guard(_lock);
  std::string leastLargerThan;
  std::string least;

  for (auto& client : _clientConnections)
  {
    const std::string& otherId = client.second->GetNetworkId();
    if ((otherId < leastLargerThan || leastLargerThan.empty()) && otherId > prev)
      leastLargerThan = client.second->GetNetworkId();

    if ((least < otherId || least.empty()) && otherId < prev)
      least = otherId;
  }

  if (!leastLargerThan.empty())
    return leastLargerThan;

  return least;
}

const std::string& NetworkManager::GetLeaderId() const
{
  return _leaderId;
}