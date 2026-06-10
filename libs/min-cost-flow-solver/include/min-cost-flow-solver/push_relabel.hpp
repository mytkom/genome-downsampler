#ifndef PUSH_RELABEL_HPP
#define PUSH_RELABEL_HPP

#include <cstdint>
#include <vector>

#include "common.hpp"
#include "min-cost-flow-solver/height_queue.hpp"

// Sequential push-relabel implementation with global relabel heuristics
// run every kGlobalRelabelFrequency*N.
namespace min_cost_flow_solver {
class PushRelabel {
    static constexpr uint64_t kExpectedMinEdgesMult = 8;
    static constexpr uint64_t kGlobalRelabelFrequency = 5;

    // Sentinel value indicating the end of an edge chain
    static constexpr EdgeIndex kInvalidEdge = static_cast<EdgeIndex>(-1);

    Index n_;
    Index source_;
    Index sink_;

    std::vector<EdgeNode> edges_;
    std::vector<VertexProperties> vertices_;
    std::vector<Index> bfs_queue_;

    HeightQueue active_;

    inline void enqueue(Index v);
    inline void push(Index u, EdgeIndex e);
    inline void relabel(Index u);
    inline void discharge(Index u);
    inline void globalRelabel();

   public:
    explicit PushRelabel(Index nodes) : n_(nodes) {
        const uint64_t exp_edge_count = kExpectedMinEdgesMult * n_;
        edges_.reserve(2 * exp_edge_count);

        vertices_.assign(n_, {kInvalidEdge, 0, 0});
        bfs_queue_.reserve(n_);
    }

    void addEdge(Index from, Index to, Flow cap);
    Flow computeMaxFlow(Index s, Index t);
    inline Flow getFlowVal(Index i) const { return edges_[i*2 + 1].res_cap; }
};

}  // namespace min_cost_flow_solver

#endif
