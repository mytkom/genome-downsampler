#ifndef MIN_COST_FLOW_SOLVER_HEIGHT_QUEUE_HPP
#define MIN_COST_FLOW_SOLVER_HEIGHT_QUEUE_HPP

#include <vector>

#include "common.hpp"

namespace min_cost_flow_solver {
class HeightQueue {
    std::vector<std::pair<Index, Height>> odd_;
    std::vector<std::pair<Index, Height>> even_;

   public:
    void push(Index v, Height h) {
      (h & 1) ? odd_.push_back(std::make_pair(v, h)) : even_.push_back(std::make_pair(v, h));
    }

    void clear() {
      odd_.clear();
      even_.clear();
    }

    void reserve(Index n) {
      odd_.reserve(n);
      even_.reserve(n);
    }

    bool is_empty() {
      return even_.empty() && odd_.empty();
    }

    Index pop() {
      if(even_.empty()) return pop_back(odd_);
      if (odd_.empty()) return pop_back(even_);

      if(even_.back().second > odd_.back().second)
        return pop_back(even_);

      return pop_back(odd_);
    }

    static Index pop_back(std::vector<std::pair<Index, Height>>& q) {
      Index v = q.back().first;
      q.pop_back();
      return v;
    }
};

}  // namespace min_cost_flow_solver

#endif
