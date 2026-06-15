#include "min-cost-flow-solver/push_relabel_simple_heuristics.hpp"

void min_cost_flow_solver::PushRelabelSimpleHeuristics::enqueue(Index v) {
    active_.push(v, height_[v]);
}

void min_cost_flow_solver::PushRelabelSimpleHeuristics::push(Index u, Edge& e) {
    Flow delta = std::min(e.cap - e.flow, excess_[u]);

    excess_[u] -= delta;
    e.flow += delta;
    excess_[e.to] += delta;
    adj_[e.to][e.rev_index].flow -= delta;

    if (e.to != sink_ && excess_[e.to] == delta) enqueue(e.to);
}

void min_cost_flow_solver::PushRelabelSimpleHeuristics::relabel(Index u) {
    work_counter_++;
    Height min_h = n_;

    for (const Edge& e : adj_[u]) {
        if (e.cap - e.flow > 0) min_h = std::min(min_h, height_[e.to]);
    }

    height_[u] = min_h + 1;
}

// Global Relabeling Heuristic
void min_cost_flow_solver::PushRelabelSimpleHeuristics::globalRelabel() {
    bfs_queue_.clear();

    for (Index i = 0; i < n_; i++)
        height_[i] = n_;
    height_[sink_] = 0;

    bfs_queue_.push_back(sink_);
    Index bfs_index = 0;

    while (bfs_index < bfs_queue_.size()) {
        Index u = bfs_queue_[bfs_index];

        for (const Edge& e : adj_[u]) {
            Edge& inv_e = adj_[e.to][e.rev_index];
            if (e.to != source_ && inv_e.cap - inv_e.flow > 0 && height_[e.to] == n_) {
                height_[e.to] = height_[u] + 1;

                if (height_[e.to] < n_ - 1) bfs_queue_.push_back(e.to);
            }
        }

        ++bfs_index;
    }

    for (Index i = 0; i < n_; ++i) {
        if (i != sink_ && i != source_ && excess_[i] > 0) enqueue(i);
    }
}

void min_cost_flow_solver::PushRelabelSimpleHeuristics::discharge(Index u) {
    auto it = adj_[u].begin();

    while (excess_[u] > 0) {
        if (it == adj_[u].cend()) {
            it = adj_[u].begin();
            relabel(u);
        } else {
            Edge& e = *it;
            if (e.cap - e.flow > 0 && height_[u] == height_[e.to] + 1) push(u, e);
            it++;
        }
    }
}

void min_cost_flow_solver::PushRelabelSimpleHeuristics::addEdge(Index from, Index to, Flow cap) {
    if (from == to) return;  // Ignore self-loops, they break reverse indexing

    original_edges_.push_back({from, adj_[from].size()});
    adj_[from].push_back({to, adj_[to].size(), cap, 0});
    adj_[to].push_back({from, adj_[from].size() - 1, 0, 0});
}

min_cost_flow_solver::Flow min_cost_flow_solver::PushRelabelSimpleHeuristics::computeMaxFlow(
    Index s, Index t) {
    source_ = s;
    sink_ = t;
    excess_.assign(n_, 0);
    height_.assign(n_, 0);
    active_.reserve(n_);

    globalRelabel();
    height_[source_] = n_;

    // Push initial flow from source
    for (auto& e : adj_[source_]) {
        if (e.cap > 0) {
            excess_[source_] += e.cap;
            push(source_, e);
        }
    }


    while (!active_.is_empty()) {
        if(work_counter_ >= work_threshold_) {
          work_counter_ = 0;
          globalRelabel();
        }

        Index u = active_.pop();
        if (u != source_ && u != sink_) {
            discharge(u);
        }
    }

    return excess_[sink_];
}

min_cost_flow_solver::Flow min_cost_flow_solver::PushRelabelSimpleHeuristics::getFlowVal(
    Index i) const {
    const auto& pair = original_edges_[i];
    const Edge& e = adj_[pair.first][pair.second];
    return e.flow;
}
