#pragma once
#include <queue>
#include "StackAssignment.h"

class Computation;

#include "MaximumCut.h"
#include "BlockingQueue.h"
#include "ReceivedMessage.h"
#include "ParallelStack.h"

class Computation
{
private:
  enum State
  {
    Initial,
    Computing,
    Waiting,
    Terminating,
    Finished
  };
  BlockingQueue<std::shared_ptr<ReceivedMessage> > _messages;
  std::shared_ptr<MaximumCut> _maximumCut;
  std::shared_ptr<std::thread> _computeLoop;
  std::shared_ptr<StackAssignment> _currAssignment;
  void ComputationLoop();
  std::vector<std::shared_ptr<ParallelStack<std::vector<int> > > > GetParallelStacks(int count);
  void InitLeaderStep();
  void LeaderStep();
  void NonLeaderStep();
  void ComputeStep();
  void OnAssignment(const std::shared_ptr<StackAssignment>& assign);
  void OnAssignmentFinished(int32_t best, const std::string& clientId);
  void OnMessage(const std::shared_ptr<ReceivedMessage>& msg);
  void ReadMessages();
  State _state;
  int32_t _bestFound;
public:
  Computation();
  ~Computation();
  void StartComputation();
  void SetMaximumCut(const std::shared_ptr<MaximumCut>& maximumCut) { _maximumCut = maximumCut; }
  const std::shared_ptr<MaximumCut>& GetMaximumCut() const { return _maximumCut; }

  void AddMessage(const std::shared_ptr<ReceivedMessage>& msg);
};
