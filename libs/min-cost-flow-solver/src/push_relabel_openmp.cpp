#include "min-cost-flow-solver/push_relabel_openmp.hpp"

#include <algorithm>
#include <atomic>
#include <cstdint>

namespace min_cost_flow_solver {

inline void PushRelabelOpenMp::enqueue(Index v) {
    omp_set_lock(&queue_lock_);
    active_.push(v, vertices_[v].height);
    omp_unset_lock(&queue_lock_);
}

inline void PushRelabelOpenMp::push(Index ui, EdgeIndex ei) {
    VertexProperties& u = vertices_[ui];
    EdgeNode& e = edges_[ei];
    EdgeNode& rev_e = edges_[ei ^ 1];
    Index vi = e.to;

    Index lock_first = std::min(ui, vi);
    Index lock_second = std::max(ui, vi);

    // note(mytkom): order important: deadlocks possible
    omp_set_lock(&vertex_locks_[lock_first]);
    omp_set_lock(&vertex_locks_[lock_second]);

    Flow delta = std::min(u.excess, e.res_cap);

    if (delta > 0) {
        e.res_cap -= delta;
        u.excess -= delta;
        rev_e.res_cap += delta;

        VertexProperties& v = vertices_[vi];

        bool should_enqueue = (vi != sink_ && v.excess == 0);
        v.excess += delta;

        if (should_enqueue) {
            enqueue(vi);
        }
    }

    // note(mytkom): order important: deadlocks possible
    omp_unset_lock(&vertex_locks_[lock_second]);
    omp_unset_lock(&vertex_locks_[lock_first]);
}

inline void PushRelabelOpenMp::relabel(Index u) {
    Height min_h = n_;

    for (EdgeIndex e = vertices_[u].head; e != kInvalidEdge; e = edges_[e].next) {
        if (edges_[e].res_cap > 0) {
            min_h = std::min(min_h, vertices_[edges_[e].to].height);
        }
    }

    // Only lock the specific vertex being relabeled
    omp_set_lock(&vertex_locks_[u]);

    vertices_[u].height = min_h + 1;
    if (vertices_[u].excess > 0) {
        enqueue(u);
    }

    omp_unset_lock(&vertex_locks_[u]);
}

inline void PushRelabelOpenMp::discharge(Index u) {
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

inline void PushRelabelOpenMp::globalRelabel() {
    bfs_queue_.clear();
    Index bfs_index = 0;

    active_.clear();

    for (auto& v : vertices_)
        if (v.height < n_) v.height = n_;

    bfs_queue_.push_back(sink_);
    vertices_[sink_].height = 0;

    // BFS from Sink
    while (bfs_index < bfs_queue_.size()) {
        Index v = bfs_queue_[bfs_index];

        Height current_height = vertices_[v].height;
        if (current_height >= n_ - 1) continue;

        EdgeIndex e = vertices_[v].head;

        while (e != kInvalidEdge) {
            const auto& edge = edges_[e];
            VertexProperties& w = vertices_[edge.to];

            if (edge.to != source_ && w.height == n_ && edges_[e ^ 1].res_cap > 0) {
                w.height = current_height + 1;
                bfs_queue_.push_back(edge.to);
                if (w.excess > 0) enqueue(edge.to);
            }

            e = edge.next;
        }

        ++bfs_index;
    }
}

void PushRelabelOpenMp::addEdge(Index from, Index to, Flow cap) {
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

Flow PushRelabelOpenMp::computeMaxFlow(Index s, Index t) {
    source_ = s;
    sink_ = t;

    active_.reserve(n_);

    // Initialize all locks
    omp_init_lock(&queue_lock_);
    vertex_locks_.resize(n_);
    for (Index i = 0; i < n_; ++i) {
        omp_init_lock(&vertex_locks_[i]);
    }

    globalRelabel();
    vertices_[source_].height = n_;

    for (EdgeIndex e = vertices_[source_].head; e != kInvalidEdge; e = edges_[e].next) {
        if (edges_[e].res_cap > 0) {
            vertices_[source_].excess += edges_[e].res_cap;
            push(source_, e);
        }
    }

    std::atomic<uint64_t> global_steps = 0;
    std::atomic<int> active_workers{0};
    bool is_done = false;

    const size_t CHUNK_SIZE = 100;

    #pragma omp parallel
    {
        uint64_t local_steps = 0;

        std::vector<Index> local_queue;
        local_queue.reserve(CHUNK_SIZE);

        while (!is_done) {
            if (global_steps >= kGlobalRelabelFrequency * n_) {
                #pragma omp barrier
                #pragma omp single
                {
                    globalRelabel();
                    global_steps = 0;
                }
            }

            // populate local queue
            local_queue.clear();
            omp_set_lock(&queue_lock_);
            while (!active_.is_empty() && local_queue.size() < CHUNK_SIZE) {
                local_queue.push_back(active_.pop());
            }
            if (!local_queue.empty()) {
                active_workers++;
            }
            omp_unset_lock(&queue_lock_);

            if (!local_queue.empty()) {
                for (Index v : local_queue) {
                    if (v != sink_ && v != source_) {
                        discharge(v);
                        ++local_steps;

                        if (local_steps > 100) {
                            global_steps += local_steps;
                            local_steps = 0;
                        }
                    }
                }
                active_workers--;
            }
            else {
                if (active_workers == 0) {
                    omp_set_lock(&queue_lock_);
                    if (active_.is_empty() && active_workers == 0) {
                        is_done = true;
                    }
                    omp_unset_lock(&queue_lock_);
                }
            }
        }
    }  // end parallel

    // clean up
    omp_destroy_lock(&queue_lock_);
    for (Index i = 0; i < n_; ++i) {
        omp_destroy_lock(&vertex_locks_[i]);
    }

    return vertices_[sink_].excess;
}

}  // namespace min_cost_flow_solver
