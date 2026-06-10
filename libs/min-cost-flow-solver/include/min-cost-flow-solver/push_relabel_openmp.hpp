#ifndef PUSH_RELABEL_OPENMP_HPP
#define PUSH_RELABEL_OPENMP_HPP

#include <cstdint>
#include <vector>
#include <omp.h>

#include "common.hpp"
#include "min-cost-flow-solver/height_queue.hpp"

namespace min_cost_flow_solver {

// Push-relabel max flow implementation with OpenMP pararellisation
// and global-relabel heuristics run every kGlobalRelabelFrequency*N.
class PushRelabelOpenMp {
    static constexpr uint64_t kExpectedMinEdgesMult = 8;
    static constexpr uint64_t kGlobalRelabelFrequency = 5;

    // Sentinel value indicating the end of an edge chain
    static constexpr EdgeIndex kInvalidEdge = static_cast<EdgeIndex>(-1);
    static constexpr Index kInvalidIndex = static_cast<Index>(-1);

    Index n_;
    Index source_;
    Index sink_;

    omp_lock_t queue_lock_;
    std::vector<omp_lock_t> vertex_locks_;
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
    explicit PushRelabelOpenMp(Index nodes) : n_(nodes) {
        const uint64_t exp_edge_count = kExpectedMinEdgesMult * n_;
        edges_.reserve(2 * exp_edge_count);

        vertices_.assign(n_, {kInvalidEdge, 0, 0});
        bfs_queue_.reserve(n_);
    }

    void addEdge(Index from, Index to, Flow cap);
    Flow computeMaxFlow(Index s, Index t);
    inline Flow getFlowVal(Index i) const { return edges_[i * 2 + 1].res_cap; }
};

}  // namespace min_cost_flow_solver

#endif
