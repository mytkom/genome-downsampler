#ifndef PUSH_RELABEL_VECTOR_SIMPLE_HEURISTICS_HPP
#define PUSH_RELABEL_VECTOR_SIMPLE_HEURISTICS_HPP

#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <queue>
#include <vector>

#include "common.hpp"
#include "logging/log.hpp"

namespace min_cost_flow_solver {

class PushRelabelVectorSimpleHeuristics {
    // we expect at kExpectedMinEdgesMult * n_ edges -> optim to reserve memory
    static constexpr uint64_t kExpectedMinEdgesMult = 8;
    // const for heuristics
    static constexpr uint64_t kHeuristicsDiv = 5;

    Index n_;
    Index source_;
    Index sink_;

    // common EdgeIndex key
    std::vector<Flow> edge_flows_;
    std::vector<Flow> edge_caps_;
    std::vector<Index> edge_to_;
    // out_edges_ for each vertex, adjacency list through edge ids
    std::vector<std::vector<EdgeIndex>> out_edges_;
    std::vector<EdgeIndex> edge_reverse_;
    std::vector<Flow> excess_;

    // TODO(mytkom): test vector of queues
    std::vector<std::vector<Index>> active_;

    // height-related
    std::vector<Height> height_;
    Height max_height_ = 0;

    // Tracks the chronological order of edges added by the user
    // Stores pairs of {from_node, index_in_adj_list}
    std::vector<std::pair<Index, Index>> original_edges_;
    inline bool isForward(EdgeIndex e) { return edge_caps_[e] > 0; }

    void enqueue(Index v);
    void push(Index u, EdgeIndex e);
    void relabel(Index u);
    void discharge(Index u);

    // Global Relabeling Heuristic
    // Heuristic triggers
    // Index work_counter_ = 0;
    // Index work_threshold_;
    // void globalRelabel();

   public:
    explicit PushRelabelVectorSimpleHeuristics(Index nodes) : n_(nodes) {
        const uint64_t exp_edge_count = kExpectedMinEdgesMult * n_;

        edge_caps_.reserve(2 * exp_edge_count);
        edge_flows_.reserve(2 * exp_edge_count);
        edge_to_.reserve(2 * exp_edge_count);
        original_edges_.reserve(exp_edge_count);

        out_edges_.resize(n_);
        for (auto& edges : out_edges_) {
            edges.reserve(2 * kExpectedMinEdgesMult);
        }
    }

    void addEdge(Index from, Index to, Flow cap);
    Flow computeMaxFlow(Index s, Index t);
    Flow getFlowVal(Index i) const;
};

}  // namespace min_cost_flow_solver

#endif
