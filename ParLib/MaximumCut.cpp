
#include "Common.h"
#include "ParallelStack.h"
#include <iostream>
#include <algorithm>
#include "MaximumCut.h"

MaximumCut::MaximumCut(const std::shared_ptr<Matrix>& matrix)
  : _bestMax(0), _matrix(matrix)
{
}

ParallelStack<std::vector<int32_t> > &MaximumCut::getStates()
{
  return _states;
}

int32_t MaximumCut::getBestMax()
{
  return _bestMax;
}

void MaximumCut::setBestMax(int32_t max)
{
  _bestMax = max;
}

void MaximumCut::PopulateStates()
{
  for (uint32_t i = 0; i < GetMatrix().getSize(); i++)
  {
    std::vector<int32_t> nodes;
    nodes.push_back(i);
    getStates().push(nodes);

    nodes.clear();
  }
}

bool MaximumCut::compute()
{
#ifdef DEBUG
  Log << myID() << "Compute" << std::endl;
#endif

  if (_matrix->getSize() < 1)
  {
    Err << "ERROR: Matrix is empty!" << std::endl;
    exit(1);
  }

  uint32_t tmp = 0;

  std::vector<int32_t> current;

#ifdef DEBUG
  /////////////////////////////////// TODO: Remove this debug

  std::chrono::milliseconds dura(5);
  std::this_thread::sleep_for(dura);

  ////////////////////////////////////
#endif

  int loopCycles = 10;

  while (!_states.empty())
  {
    current = _states.top();
    _states.pop();

    int32_t item = current.back();

    for (auto neigh : _matrix->getNeigh(item))
    {
      int32_t n = neigh;
      if (std::find(current.begin(), current.end(), n) == current.end())
      {
        current.push_back(neigh);
        _states.push(current);
        current.pop_back();
      }
    }

    tmp = _matrix->sum(current);
    if (tmp > _bestMax)
    {
      _bestMax = tmp;
    }
    if (loopCycles-- == 0)
    {
      return false;
    }
  }
  return true;
}
