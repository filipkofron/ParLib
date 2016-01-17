#pragma once

class MaximumCut;

#include <memory>
#include "Matrix.h"
#include "ParallelStack.h"

class MaximumCut
{
protected:
  ParallelStack<std::vector<int32_t> > _states;
  uint32_t _bestMax;
  std::shared_ptr<Matrix> _matrix;
public:
  MaximumCut(const std::shared_ptr<Matrix>& matrix);
  bool compute();
  ParallelStack<std::vector<int32_t> >& getStates();
  int32_t getBestMax();
  void setBestMax(int32_t max);
  Matrix& GetMatrix() const { return *_matrix; }

  void PopulateStates();
};
