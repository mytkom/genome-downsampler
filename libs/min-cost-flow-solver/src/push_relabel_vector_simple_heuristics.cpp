#include "min-cost-flow-solver/push_relabel_vector_simple_heuristics.hpp"

#include <utility>

void min_cost_flow_solver::PushRelabelVectorSimpleHeuristics::enqueue(Index v) {
    active_[height_[v]].push_back(v);
    max_height_ = std::max(max_height_, height_[v]);
}

void min_cost_flow_solver::PushRelabelVectorSimpleHeuristics::push(Index u, EdgeIndex e) {
    Flow delta = std::min(excess_[u], edge_caps_[e] - edge_flows_[e]);

    EdgeIndex rev_edge = edge_reverse_[e];
    edge_flows_[e] += delta;
    excess_[u] -= delta;
    edge_flows_[rev_edge] -= delta;
    excess_[edge_to_[e]] += delta;

    // enqueue neighbor if it has become active in this step
    if (edge_to_[e] != sink_ && excess_[edge_to_[e]] == delta) enqueue(edge_to_[e]);
}

void min_cost_flow_solver::PushRelabelVectorSimpleHeuristics::relabel(Index u) {
    Height min_h = n_;

    for (const EdgeIndex& e : out_edges_[u]) {
        if (edge_caps_[e] - edge_flows_[e] > 0) min_h = std::min(min_h, height_[edge_to_[e]]);
    }

    height_[u] = min_h + 1;
    max_height_ = std::max(max_height_, height_[u]);
}

void min_cost_flow_solver::PushRelabelVectorSimpleHeuristics::discharge(Index u) {
    auto it = out_edges_[u].begin();

    while (excess_[u] > 0) {
        if (it == out_edges_[u].cend()) {
            it = out_edges_[u].begin();
            relabel(u);
        } else {
            if (edge_caps_[*it] - edge_flows_[*it] > 0 && height_[u] == height_[edge_to_[*it]] + 1)
                push(u, *it);
            ++it;
        }
    }
}

void min_cost_flow_solver::PushRelabelVectorSimpleHeuristics::addEdge(Index from, Index to,
                                                                     Flow cap) {
    if (from == to) return;

    // save original pair for flow reconstruction
    original_edges_.push_back(std::make_pair(from, out_edges_[from].size()));

    // add forward edge
    EdgeIndex forward_eid = edge_to_.size();
    out_edges_[from].push_back(forward_eid);
    edge_to_.push_back(to);
    edge_flows_.push_back(0);
    edge_caps_.push_back(cap);

    // add backward edge
    out_edges_[to].push_back(forward_eid + 1);  // convention
    edge_to_.push_back(from);
    edge_flows_.push_back(0);
    edge_caps_.push_back(0);
}

min_cost_flow_solver::Flow min_cost_flow_solver::PushRelabelVectorSimpleHeuristics::computeMaxFlow(
    Index s, Index t) {
    source_ = s;
    sink_ = t;

    //  Rebuild edge arrays to better utilise cache:
    {
      const Index num_nodes = out_edges_.size();
      const Index num_edges = edge_to_.size();

      // 1. Establish the vertex order: source, intermediate nodes, sink
      std::vector<Index> vertex_order;
      vertex_order.reserve(num_nodes);

      vertex_order.push_back(s);
      for (Index v = 0; v < num_nodes; ++v) {
          if (v != s && v != t) {
              vertex_order.push_back(v);
          }
      }
      if (s != t) {
          vertex_order.push_back(t);
      }

      // 2. Prepare temporary buffers for the new layout
      std::vector<Index> new_edge_to;
      std::vector<Flow> new_edge_flows;
      std::vector<Flow> new_edge_caps;
      new_edge_to.reserve(num_edges);
      new_edge_flows.reserve(num_edges);
      new_edge_caps.reserve(num_edges);

      std::vector<std::vector<EdgeIndex>> new_out_edges(num_nodes);

      // Translation map: old_to_new[old_edge_id] = new_edge_id
      std::vector<EdgeIndex> old_to_new(num_edges);

      // 3. Populate new contiguous arrays and record ID translations
      for (Index v : vertex_order) {
          new_out_edges.reserve(out_edges_[v].size());

          for (EdgeIndex old_eid : out_edges_[v]) {
              EdgeIndex new_eid = new_edge_to.size();
              old_to_new[old_eid] = new_eid; 

              new_edge_to.push_back(edge_to_[old_eid]);
              new_edge_flows.push_back(edge_flows_[old_eid]);
              new_edge_caps.push_back(edge_caps_[old_eid]);

              new_out_edges[v].push_back(new_eid);
          }
      }

      // 4. Encode the O(1) reverse edge lookups using the tracking data
      std::vector<EdgeIndex> new_edge_reverse(num_edges);
      for (Index v = 0; v < num_nodes; ++v) {
          for (EdgeIndex old_eid : out_edges_[v]) {
              // The original convention guarantees that forward/backward pairs were (0,1), (2,3), etc.
              // Therefore: old_reverse_id = old_id XOR 1
              EdgeIndex old_reverse_eid = old_eid ^ 1; 

              EdgeIndex current_new_eid = old_to_new[old_eid];
              EdgeIndex reverse_new_eid = old_to_new[old_reverse_eid];

              new_edge_reverse[current_new_eid] = reverse_new_eid;
          }
      }

      // 5. Commit shifts to class members
      edge_to_ = std::move(new_edge_to);
      edge_flows_ = std::move(new_edge_flows);
      edge_caps_ = std::move(new_edge_caps);
      out_edges_ = std::move(new_out_edges);
      edge_reverse_ = std::move(new_edge_reverse);
    }


    // initialise containers
    excess_.assign(n_, 0);
    height_.assign(n_, 0);
    active_.assign(2 * n_, std::vector<Index>());
    for (Index i = 0; i < active_.size(); ++i) active_[i].reserve(kExpectedMinEdgesMult);

    max_height_ = 0;
    for (Index i = 0; i < n_ - 2; ++i) height_[i] = (n_ - i) / kHeuristicsDiv;
    height_[source_] = n_;

    // Step 1: saturate all out of source edges
    for (const EdgeIndex& e : out_edges_[source_]) {
        if (isForward(e)) {
            excess_[source_] += edge_caps_[e];
            push(source_, e);
        }
    }

    // Step 2: push-relabel
    while (true) {
        if (active_[max_height_].empty()) {
            // termination, no more active vertices
            if (max_height_ == 0) break;

            max_height_--;
            continue;
        }

        // get active vertex with max height
        Index v = active_[max_height_].back();
        // disactivate, it will be processed until excess[v] == 0 -> no longer active
        active_[max_height_].pop_back();

        if (v != sink_ && v != source_) discharge(v);
    }

    return excess_[sink_];
}

min_cost_flow_solver::Flow min_cost_flow_solver::PushRelabelVectorSimpleHeuristics::getFlowVal(
    Index i) const {
    const auto& [j, k] = original_edges_[i];
    const EdgeIndex& e = out_edges_[j][k];
    return edge_flows_[e];
}
