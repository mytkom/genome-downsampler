#ifndef PUSH_RELABEL_SIMPLE_HEURISTICS_HPP
#define PUSH_RELABEL_SIMPLE_HEURISTICS_HPP

#include <vector>

#include "common.hpp"
#include "min-cost-flow-solver/height_queue.hpp"

namespace min_cost_flow_solver {
// Internal structure for the residual graph
struct alignas(64) Edge {
    Index to;
    Index rev_index;
    Flow cap;
    Flow flow;
};

class PushRelabelSimpleHeuristics {
    Index n_;
    Index source_;
    Index sink_;
    std::vector<Index> bfs_queue_;
    std::vector<std::vector<Edge>> adj_;
    std::vector<Flow> excess_;
    std::vector<Height> height_;
    HeightQueue active_;

    // Tracks the chronological order of edges added by the user
    // Stores pairs of {from_node, index_in_adj_list}
    std::vector<std::pair<Index, Index>> original_edges_;

    // Heuristic triggers
    Index work_counter_ = 0;
    Index work_threshold_;

    void enqueue(Index v);
    void push(Index u, Edge& e);
    void relabel(Index u);

    // Global Relabeling Heuristic
    void globalRelabel();
    void discharge(Index u);

   public:
    explicit PushRelabelSimpleHeuristics(Index nodes)
        : n_(nodes),
          adj_(nodes),
          work_threshold_(nodes) {
      bfs_queue_.reserve(nodes);
    }

    void addEdge(Index from, Index to, Flow cap);
    Flow computeMaxFlow(Index s, Index t);
    Flow getFlowVal(Index i) const;
};

}  // namespace min_cost_flow_solver

#endif
