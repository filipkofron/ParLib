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
    ReAssignment,
    Waiting,
    Terminating,
    Finished
  };
  struct ReturnedStack
  {
    std::string id;
    std::vector<std::vector<int> > stack;
  };
  BlockingQueue<std::shared_ptr<ReceivedMessage> > _messages;
  std::shared_ptr<MaximumCut> _maximumCut;
  std::shared_ptr<std::thread> _computeLoop;
  std::shared_ptr<StackAssignment> _currAssignment;
  std::vector<ReturnedStack> _returnedStacks;
  std::vector<ReturnedStack> _UnIdreturnedStacks;
  void ComputationLoop();
  std::vector<std::shared_ptr<ParallelStack<std::vector<int> > > > GetParallelStacks(int count);
  std::vector<std::shared_ptr<ParallelStack<std::vector<int> > > > GetReturnedParallelStacks(int count);
  void InitLeaderStep();
  void CheckAssignments();
  void LeaderStep();
  void NonLeaderStep();
  void ComputeStep();
  void OnAssignment(const std::shared_ptr<StackAssignment>& assign);
  void OnAssignmentFinished(int32_t best, const std::string& clientId);
  void OnTerminate();
  void OnRequestReturnStack();
  void OnRequestReturningStack(const std::string&sender, const std::vector<std::vector<int>>& vec, int32_t best);
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
  void Terminate();
  bool HasFinished();
};
