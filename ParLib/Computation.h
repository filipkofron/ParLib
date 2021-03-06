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
  std::set<std::string> GetAssignedClients() const;
  void ComputationLoop();
  std::vector<std::shared_ptr<ParallelStack<std::vector<int> > > > GetParallelStacks(const std::vector<std::vector<int> > states, int count);
  void InitLeaderStep();
  void OnDivideWith(const std::string& with);
  void CheckAssignments();
  void ClearDisconnected();
  void LeaderStep();
  void NonLeaderStep();
  void ComputeStep();
  void OnAssignment(const std::shared_ptr<StackAssignment>& assign);
  void OnAssignmentFinished(int32_t best, const std::string& clientId);
  void OnTerminate();
  void OnMessage(const std::shared_ptr<ReceivedMessage>& msg);
  void ReadMessages();
  State _state;
  int32_t _bestFound;
  int64_t _lastAssignCheck;
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
