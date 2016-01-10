#include "Computation.h"
#include <iostream>
#include "StackAssignment.h"
#include "MessageFactory.h"

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

std::vector<std::shared_ptr<ParallelStack<std::vector<int> > > > Computation::GetParallelStacks(int count)
{
  std::vector<std::shared_ptr<ParallelStack<std::vector<int> > > > pars;
  std::vector<std::shared_ptr<ParallelStack<std::vector<int> > > > newPars;
  
  pars.push_back(std::make_shared<ParallelStack<std::vector<int> > >());
  auto origStates = _maximumCut->getStates().getData();
  pars[0]->assignData(origStates);
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

std::vector<std::shared_ptr<ParallelStack<std::vector<int> > > > Computation::GetReturnedParallelStacks(int count)
{
  std::vector<std::shared_ptr<ParallelStack<std::vector<int> > > > pars;
  std::vector<std::shared_ptr<ParallelStack<std::vector<int> > > > newPars;

  int toDelete = 0;
  for (int i = 0; i < count && i < _returnedStacks.size(); i++)
  {
    auto par = std::make_shared<ParallelStack<std::vector<int> > >();
    par->assignData(_returnedStacks[i].stack);
    pars.push_back(par);
    toDelete++;
  }
  std::vector<ReturnedStack> newStacks;
  for (int i = toDelete; i < _returnedStacks.size(); i++)
  {
    newStacks.push_back(_returnedStacks[i]);
  }
  _returnedStacks = newStacks;
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

  std::vector<std::shared_ptr<ParallelStack<std::vector<int> > > > pars = GetParallelStacks(count);
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

void Computation::CheckAssignments()
{
  auto net = GNetworkManager;
  if (!net)
    return;

  auto clients = net->GetClients();
  auto assClients = _currAssignment->GetClients();
  std::set<std::string> refreshOrNewClients;
  std::set<std::string> workingClients;
  std::vector<std::string> lostClients;

  for (auto& assign : assClients)
  {
    if (!net->FindClient(assign))
    {
      lostClients.push_back(assign);
    }
    if (_currAssignment->GetAssignment(assign).size() == 0)
    {
      refreshOrNewClients.insert(assign);
    }
    else
    {
      workingClients.insert(assign);
    }
  }
  for (auto& assign : _returnedStacks)
  {
    refreshOrNewClients.insert(assign.id);
    workingClients.erase(assign.id);
  }
  int count = static_cast<int>(clients.size());
  for (auto& client : clients)
  {
    if (_currAssignment->GetAssignment(client).size() == 0)
    {
      refreshOrNewClients.insert(client);
    }
  }
  if (refreshOrNewClients.size() > 0)
  {
    for (auto& toRefresh : workingClients)
    {
      auto msg = MessageFactory::CreateRequestReturnStackMessage();
      net->SendMsg(*msg, toRefresh);
    }
  }
  for (auto& assign : _returnedStacks)
  {
    refreshOrNewClients.erase(assign.id);
  }
  auto stackAss = std::make_shared<StackAssignment>();

  for (auto& toRefresh : workingClients)
  {
    stackAss->GetAssignment(toRefresh) = _currAssignment->GetAssignment(toRefresh);
  }
  // std::cout << "Working clients: " << workingClients.size() << std::endl;
  if (workingClients.size() == 0)
  {
    auto msg = MessageFactory::CreateTerminateMessage();
    net->BroadcastMsg(*msg);
    std::cout << "[Leader] Terminating, best solution found: " << _bestFound << std::endl;
    OnTerminate();
  }
  if (refreshOrNewClients.size() == 0)
    return;

  auto newStacks = GetReturnedParallelStacks(static_cast<int>(refreshOrNewClients.size()));
  int test = 0;
  for (auto& toRefresh : refreshOrNewClients)
  {
    if (test >= newStacks.size())
      return;
    stackAss->GetAssignment(toRefresh) = newStacks[test++]->getData();
  }

  OnAssignment(stackAss);

  auto msg = MessageFactory::CreateStackAssignmentMessage(*stackAss);

  for (auto& toRefresh : refreshOrNewClients)
  {
    net->SendMsg(*msg, toRefresh);
  }
  for (auto& toRefresh : workingClients)
  {
    net->SendMsg(*msg, toRefresh);
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
  if (_state == ReAssignment)
  {
    if (_currAssignment->Equals(GNetworkManager->GetNetworkId(), assign))
    {
      _state = Waiting;
    }
    else
    {
      return;
    }
  }
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
  if (best > _bestFound)
  {
    _bestFound = best;
    std::cout << "Best found by '" << clientId << "': " << best << std::endl;
  }
  auto& assign = _currAssignment->GetAssignment(clientId);
  if (assign.size() > 0)
  {
    assign.clear();
  }
}

void Computation::OnTerminate()
{
  std::cout << "Termination request from leader, terminating this node." << std::endl;
  _state = Finished;
}

void Computation::OnRequestReturnStack()
{
  if (_state == Computing)
  {
    _state = ReAssignment;
    auto msg = MessageFactory::CreateReturningStackMessage(_maximumCut->getStates().getData(), _maximumCut->getBestMax());
    GNetworkManager->SendMsg(*msg, GNetworkManager->GetLeaderId());
  }
}

void Computation::OnRequestReturningStack(const std::string& sender, const std::vector<std::vector<int>>& vec, int32_t best)
{
  if (best > _bestFound)
    _bestFound = best;

  _returnedStacks.push_back({ sender, vec });
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
  } else if (type == MESSAGE_TYPE_REQUEST_RETURN_STACK)
  {
    OnRequestReturnStack();
  } else if (type == MESSAGE_TYPE_RETURNING_STACK)
  {
    auto vec = bis.NextIntArrayArray();
    auto best = bis.NextInt32();
    OnRequestReturningStack(msg->GetSenderId(), vec, best);
  }
  else if (type == MESSAGE_TYPE_TERMINATE)
  {
    OnTerminate();
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
  : _state(State::Initial), _bestFound(0)
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
