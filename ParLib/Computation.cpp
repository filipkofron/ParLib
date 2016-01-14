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
