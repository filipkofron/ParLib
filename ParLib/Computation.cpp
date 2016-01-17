#include "Computation.h"
#include <iostream>
#include "StackAssignment.h"
#include "MessageFactory.h"
#include <iterator>
#include <algorithm>

std::set<std::string> Computation::GetAssignedClients() const
{
  std::set<std::string> res;
  if (!_currAssignment)
    return res;

  for (auto& assign : _currAssignment->GetClients())
  {
    res.insert(assign);
  }
  
  return res;
}

void Computation::ComputationLoop()
{
  auto net = GNetworkManager;
  while (_state != State::Finished)
  {
    net = GNetworkManager;

    if (!net)
      return;

    if (net->IsLeader())
      LeaderStep();
    else
      NonLeaderStep();

    ComputeStep();

    ReadMessages();

    if (DEBUGVerbose) std::cout << "Compute loop tick" << std::endl;
  }
}

std::vector<std::shared_ptr<ParallelStack<std::vector<int> > > > Computation::GetParallelStacks(const std::vector<std::vector<int> > states, int count)
{
  std::vector<std::shared_ptr<ParallelStack<std::vector<int> > > > pars;
  std::vector<std::shared_ptr<ParallelStack<std::vector<int> > > > newPars;
  
  pars.push_back(std::make_shared<ParallelStack<std::vector<int> > >());
  pars[0]->assignData(states);
  newPars = pars;

  while (pars.size() && pars.size() < count)
  {
    for (auto& stack : pars)
    {
      auto newStack = stack->giveInterleaved();
      newPars.push_back(newStack);
      if (newPars.size() == count)
      {
        return newPars;
      }
    }
    pars = newPars;
  }
  return pars;
}

void Computation::InitLeaderStep()
{
  auto net = GNetworkManager;
  if (!net)
    return;
  std::cout << "InitLeaderStep()" << std::endl;
  _maximumCut->PopulateStates();
  std::shared_ptr<StackAssignment> assign = std::make_shared<StackAssignment>();
  auto clients = GNetworkManager->GetClients();
  clients.push_back(GNetworkManager->GetNetworkId());
  int count = static_cast<int>(clients.size());

  for (int i = 0; i < 5; i++)
    ComputeStep();

  std::vector<std::shared_ptr<ParallelStack<std::vector<int> > > > pars = GetParallelStacks(_maximumCut->getStates().getData(), count);
  if (pars.size() != count)
  {
    FatalError("Error division!");
  }
  for (int i = 0; i < count; i++)
  {
    assign->GetAssignment(clients[i]) = pars[i]->getData();
  }
  
  OnAssignment(assign);

  auto msg = MessageFactory::CreateStackAssignmentMessage(*assign);
  for (int i = 0; i < count; i++)
  {
    net->SendMsg(*msg, clients[i]);
  }
}

void Computation::OnDivideWith(const std::string& with)
{
  std::cout << "Divide with: " << with << std::endl;
  auto parStacks = GetParallelStacks(_maximumCut->getStates().getData(), 2);
  if (parStacks.size() != 2 || parStacks[0]->size() == 0)
  {
    std::cout << "OnDivideWith: Too little work to do" << std::endl;
    return;
  }
  _maximumCut->getStates().assignData(parStacks[0]->getData());

  auto assign = std::make_shared<StackAssignment>();

  assign->GetAssignment(GNetworkManager->GetNetworkId()) = parStacks[0]->getData();
  assign->GetAssignment(with) = parStacks[1]->getData();

  auto msg = MessageFactory::CreateStackAssignmentMessage(*assign);
  GNetworkManager->BroadcastMsg(*msg);
  OnAssignment(assign);
}

void Computation::CheckAssignments()
{
  if (millis() - _lastAssignCheck < 150)
    return;

  _lastAssignCheck = millis();

  auto net = GNetworkManager;
  if (!net)
    return;

  if (_currAssignment)
  {
    std::cout << "Current assignments: " << std::endl;
    std::cout << "Me: " << GNetworkManager->GetNetworkId() << std::endl;
    _currAssignment->PrintAssignments();
  }

  std::set<std::string> knownClients;
  std::set<std::string> assignedClients = GetAssignedClients();

  // Fill known clients
  {
    auto knownClientsVec = GNetworkManager->GetClients();
    knownClientsVec.push_back(GNetworkManager->GetNetworkId());
    for (const auto& client : knownClientsVec)
      knownClients.insert(client);
  }
  
  std::set<std::string> newClients;
  std::set_difference(
    knownClients.begin(), knownClients.end(),
    assignedClients.begin(), assignedClients.end(),
    std::inserter(newClients, newClients.begin()));

  std::set<std::string> disconnectedClients;
  std::set_difference(
    assignedClients.begin(), assignedClients.end(),
    knownClients.begin(), knownClients.end(),
    std::inserter(disconnectedClients, disconnectedClients.begin()));

  std::set<std::string> finishedClients;
  for (const auto& assign : assignedClients)
  {
    if (_currAssignment->GetAssignment(assign).size() == 0)
      finishedClients.insert(assign);
  }

  // === Clear disconnected clients with no jobs ===
  {
    std::vector<std::string> clearedDisconnected;
    for (const auto& disc : disconnectedClients)
    {
      if (_currAssignment)
      {
        if (_currAssignment->GetAssignment(disc).size() == 0)
        {
          _currAssignment->Remove(disc);
          finishedClients.erase(disc);
          clearedDisconnected.push_back(disc);
        }
      }
    }
    for (const auto& disc : clearedDisconnected)
    {
      disconnectedClients.erase(disc);
    }
  }

  std::set<std::string> needJobClients;
  needJobClients = newClients;
  for (const auto& finished : finishedClients)
    needJobClients.insert(finished);

  std::set<std::string> workingClients;
  workingClients = knownClients;
  for (const auto& needJob : needJobClients)
    workingClients.erase(needJob);

  // === Structures correctly filled ===

  for (auto finished : finishedClients)
  {
    std::cout << "Finished: " << finished << std::endl;
  }

  for (auto newc : newClients)
  {
    std::cout << "New: " << newc << std::endl;
  }

  for (auto disc : disconnectedClients)
  {
    std::cout << "Disconnected: " << disc << std::endl;
  }

  for (auto known : knownClients)
  {
    std::cout << "Known: " << known << std::endl;
  }

  for (auto need : needJobClients)
  {
    std::cout << "Needs job: " << need << std::endl;
  }

  // == Try synchronize corrupted clients ==
  if ((rand() % 6) == 0)
  {
    auto msg = MessageFactory::CreateStackAssignmentMessage(*_currAssignment);
    net->BroadcastMsg(*msg);
  }

  // == Send assignments to new clients ==

  for (const auto& client : newClients)
  {
    auto msg = MessageFactory::CreateStackAssignmentMessage(*_currAssignment);
    net->SendMsg(*msg, client);
  }

  // == Terminate calculation when no jobs are left ==

  if (finishedClients.size() + newClients.size() == knownClients.size())
  {
    // only when no left over job is present we can end the computation
    if (disconnectedClients.size() == 0)
    {
      auto msg = MessageFactory::CreateTerminateMessage();
      net->BroadcastMsg(*msg);
      OnTerminate();
    }
  }

  // === Give jobs for those that need it ===

  std::shared_ptr<StackAssignment> newAssignments = std::make_shared<StackAssignment>();

  // == Give jobs from disconnected clients ==
  {
    int assignmentsNum = 0;
    while (disconnectedClients.size() > 0 && needJobClients.size() > 0)
    {
      assignmentsNum++;
      std::string givenJobTo;
      std::string givenJobFrom;
      for (const auto& job : disconnectedClients)
      {
        givenJobFrom = job;
        break;
      }
      for (const auto& job : needJobClients)
      {
        givenJobTo = job;
        break;
      }
      disconnectedClients.erase(givenJobFrom);
      needJobClients.erase(givenJobTo);      
      newAssignments->GetAssignment(givenJobTo) = _currAssignment->GetAssignment(givenJobFrom);
      newAssignments->GetAssignment(givenJobFrom) = std::vector<std::vector<int> >();
    }
    if (assignmentsNum > 0)
    {
      OnAssignment(newAssignments);

      auto msg = MessageFactory::CreateStackAssignmentMessage(*newAssignments);
      net->BroadcastMsg(*msg);
    }
  }

  for (const auto& workingClient : workingClients)
  {
    if (needJobClients.size() == 0)
      break;

    std::string needsJob;
    for (const auto& needClient : needJobClients)
    {
      needsJob = needClient;
      break;
    }
    needJobClients.erase(needsJob);

    auto msg = MessageFactory::CreateDivideWithMessage(needsJob);
    if (workingClient != GNetworkManager->GetNetworkId())
      net->SendMsg(*msg, workingClient);
    else
      OnDivideWith(needsJob);
  }
}

void Computation::ClearDisconnected()
{
  if (millis() - _lastAssignCheck < 150)
    return;

  _lastAssignCheck = millis();

  if (_currAssignment)
  {
    std::cout << "Current assignments: " << std::endl;
    std::cout << "Me: " << GNetworkManager->GetNetworkId() << std::endl;
    _currAssignment->PrintAssignments();
  }

  auto knownClientsVec = GNetworkManager->GetClients();
  knownClientsVec.push_back(GNetworkManager->GetNetworkId());
  std::set<std::string> knownClients;
  std::set<std::string> assignedClients = GetAssignedClients();

  for (const auto& client : knownClientsVec)
    knownClients.insert(client);

  std::set<std::string> disconnectedClients;
  std::set_difference(
    assignedClients.begin(), assignedClients.end(),
    knownClients.begin(), knownClients.end(),
    std::inserter(disconnectedClients, disconnectedClients.begin()));

  for (const auto& disc : disconnectedClients)
  {
    if (_currAssignment)
    {
      if (_currAssignment->GetAssignment(disc).size() == 0)
        _currAssignment->Remove(disc);
    }
  }
}

void Computation::LeaderStep()
{
  if (DEBUGVerbose) std::cout << "LeaderStep()" << std::endl;
  switch (_state)
  {
  case State::Initial:
    _state = State::Waiting;
    InitLeaderStep();
    break;
  case Terminating: break;
  case Finished: break;
  case Computing:
  case Waiting:
    CheckAssignments();
    break;
  default: break;
  }
}

void Computation::NonLeaderStep()
{
  switch (_state)
  {
  case State::Initial:
    _state = State::Waiting;
    break;
  case Terminating: break;
  case Finished: break;
  case Computing:
    break;
  case Waiting:
    break;
  default: break;
  }
  ClearDisconnected();
}

void Computation::ComputeStep()
{
  if (_maximumCut->compute())
  {
    if (_state == Computing)
    {
      // just finished computation
      OnAssignmentFinished(_maximumCut->getBestMax(), GNetworkManager->GetNetworkId());
      auto msg = MessageFactory::CreateAssignmentFinishedMessage(_maximumCut->getBestMax());
      GNetworkManager->BroadcastMsg(*msg);
      _state = Waiting;
    }
    sleepMs(50); // not doing any work, sleep instead
  }
  else
  {
    // std::cout << "Computing!" << std::endl;
  }
}

void Computation::OnAssignment(const std::shared_ptr<StackAssignment>& assign)
{
  std::cout << "Assignment came - contents " << assign->GetSize() << " clients" << std::endl;

  if (_currAssignment)
    _currAssignment->Update(assign);
  else
    _currAssignment = assign;


  if (_state != Computing)
  {
    _maximumCut->getStates().assignData(assign->GetAssignment(GNetworkManager->GetNetworkId()));
    if (_maximumCut->getStates().size() > 0)
    {
      _state = Computing;
    }
    else
    {
      _state = Waiting;
    }
    std::cout << "Got new work size: " << _maximumCut->getStates().size() << std::endl;
  }

  // compare assignments
  // _currAssignment = assign;

  // TODO: Add to computation thingy
}

void Computation::OnAssignmentFinished(int32_t best, const std::string& clientId)
{
  std::cout << "Announcing some max found by '" << clientId << "': " << best << std::endl;
  DebugFile("[%s] Assignment finished from [%s] with [%i]", GNetworkManager->GetNetworkId().c_str(), clientId.c_str(), best);
  if (best > _bestFound)
  {
    _bestFound = best;
    std::cout << "Best found by '" << clientId << "': " << best << std::endl;
  }
  _currAssignment->GetAssignment(clientId).clear();
}

void Computation::OnTerminate()
{
  std::cout << "Termination request from leader, terminating this node, max: " << _bestFound << std::endl;
  _state = Finished;
}

void Computation::OnMessage(const std::shared_ptr<ReceivedMessage>& msg)
{
  int type = msg->GetMsg()->GetType();
  ByteInputStream bis(reinterpret_cast<uint8_t*>(msg->GetMsg()->GetData()));
  if (type == MESSAGE_TYPE_STACK_ASSIGNMENT)
  {
    std::shared_ptr<StackAssignment> assign = std::make_shared<StackAssignment>();
    assign->Deserialize(bis);
    OnAssignment(assign);
  } else if (type == MESSAGE_TYPE_ASSIGNMENT_FINISHED)
  {
    int best = bis.NextInt32();
    OnAssignmentFinished(best, msg->GetSenderId());
  } else if (type == MESSAGE_TYPE_TERMINATE)
  {
    OnTerminate();
  } else if (type == MESSAGE_TYPE_DIVIDE_WITH)
  {
    OnDivideWith(bis.NextString());
  }
}

void Computation::ReadMessages()
{
  while (auto msg = _messages.PopNonBlock())
  {
    OnMessage(msg);
  }
}

Computation::Computation()
  : _state(State::Initial), _bestFound(0), _lastAssignCheck(millis())
{
}

Computation::~Computation()
{
  _state = State::Finished;
}

void Computation::StartComputation()
{
  _computeLoop = std::make_shared<std::thread>(&Computation::ComputationLoop, this);
}

void Computation::AddMessage(const std::shared_ptr<ReceivedMessage>& msg)
{
  _messages.Push(msg);
}

void Computation::Terminate()
{
  _state = State::Finished;
  _computeLoop->join();
}

bool Computation::HasFinished()
{
  return _state == Finished;
}
