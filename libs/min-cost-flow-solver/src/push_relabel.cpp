#include "min-cost-flow-solver/push_relabel.hpp"

#include <algorithm>

namespace min_cost_flow_solver {

inline void PushRelabel::enqueue(Index v) {
    active_.push(v, vertices_[v].height);
}

inline void PushRelabel::push(Index ui, EdgeIndex ei) {
    VertexProperties& u = vertices_[ui];
    EdgeNode& e = edges_[ei];
    EdgeNode& rev_e = edges_[ei^1];

    Flow delta = std::min(u.excess, e.res_cap);

    e.res_cap -= delta;
    u.excess -= delta;
    rev_e.res_cap += delta;

    VertexProperties& v = vertices_[e.to];
    v.excess += delta;

    if (e.to != sink_ && v.excess == delta) {
        enqueue(e.to);
    }
}

inline void PushRelabel::relabel(Index u) {
    Height min_h = n_;

    for (EdgeIndex e = vertices_[u].head; e != kInvalidEdge; e = edges_[e].next) {
        if (edges_[e].res_cap > 0) {
            min_h = std::min(min_h, vertices_[edges_[e].to].height);
        }
    }

    vertices_[u].height = min_h + 1;
    if(vertices_[u].excess > 0) enqueue(u);
}

inline void PushRelabel::discharge(Index u) {
    EdgeIndex curr_e = vertices_[u].head;

    while (vertices_[u].excess > 0) {
        if (curr_e == kInvalidEdge) {
            relabel(u);
            curr_e = vertices_[u].head;
        } else {
            if (vertices_[u].height == vertices_[edges_[curr_e].to].height + 1 &&
                edges_[curr_e].res_cap > 0) {
                push(u, curr_e);
            } else {
                curr_e = edges_[curr_e].next;
            }
        }
    }
}

inline void PushRelabel::globalRelabel() {
  bfs_queue_.clear();
  Index bfs_index = 0;

  active_.clear();

  for(auto& v : vertices_)
    if(v.height < n_) v.height = n_;

  bfs_queue_.push_back(sink_);
  vertices_[sink_].height = 0;

  // BFS from Sink
  while(bfs_index < bfs_queue_.size()) {
    Index v = bfs_queue_[bfs_index];

    Height current_height = vertices_[v].height;
    if(current_height >= n_ - 1) continue;

    EdgeIndex e = vertices_[v].head;

    while(e != kInvalidEdge) {
      const auto& edge = edges_[e];
      VertexProperties& w = vertices_[edge.to];

      if (edge.to != source_ && w.height == n_ && edges_[e^1].res_cap > 0) {
        w.height = current_height + 1;
        bfs_queue_.push_back(edge.to);
        if (w.excess > 0) enqueue(edge.to);
      }

      e = edge.next;
    }

    ++bfs_index;
  }
}

void PushRelabel::addEdge(Index from, Index to, Flow cap) {
    if (from == to) return;

    EdgeIndex forward_eid = edges_.size();
    EdgeIndex backward_eid = forward_eid + 1;

    // Add forward edge
    edges_.push_back({to, vertices_[from].head, cap});
    vertices_[from].head = forward_eid;

    // Add backward edge with cap 0
    edges_.push_back({from, vertices_[to].head, 0});
    vertices_[to].head = backward_eid;
}

Flow PushRelabel::computeMaxFlow(Index s, Index t) {
    source_ = s;
    sink_ = t;

    active_.reserve(n_);

    globalRelabel();
    vertices_[source_].height = n_;

    // Step 1: Saturate all outgoing edges from the source node
    for (EdgeIndex e = vertices_[source_].head; e != kInvalidEdge; e = edges_[e].next) {
        if (edges_[e].res_cap > 0) {
            vertices_[source_].excess += edges_[e].res_cap;
            push(source_, e);
        }
    }

    // Step 2: Main Push-Relabel Loop
    Index steps = 0;
    while (!active_.is_empty()) {
        if(steps >= kGlobalRelabelFrequency*n_) {
          globalRelabel();
          steps = 0;
        }

        Index v = active_.pop();
        if (v != sink_ && v != source_) {
            discharge(v);
            ++steps;
        }
    }

    return vertices_[sink_].excess;
}

}  // namespace min_cost_flow_solver
