#include "../include/qmcp-solver/quasi_mcp_cpu_own_max_flow_solver.hpp"

#include <limits>
#include <memory>
#include <numeric>
#include <algorithm>

#include "bam-api/region_api.hpp"
#include "logging/log.hpp"
#include "min-cost-flow-solver/common.hpp"
#include "min-cost-flow-solver/push_relabel_simple_heuristics.hpp"
#include "min-cost-flow-solver/push_relabel.hpp"
#include "min-cost-flow-solver/push_relabel_openmp.hpp"
#include "min-cost-flow-solver/push_relabel_vector_simple_heuristics.hpp"
#include "qmcp-solver/solver.hpp"

namespace qmcp {
  template class QuasiMcpCpuOwnMaxFlowSolver<min_cost_flow_solver::PushRelabel>;
  template class QuasiMcpCpuOwnMaxFlowSolver<min_cost_flow_solver::PushRelabelOpenMp>;
  template class QuasiMcpCpuOwnMaxFlowSolver<min_cost_flow_solver::PushRelabelSimpleHeuristics>;
  template class QuasiMcpCpuOwnMaxFlowSolver<min_cost_flow_solver::PushRelabelVectorSimpleHeuristics>;
}

template <typename T>
std::unique_ptr<qmcp::Solution> qmcp::QuasiMcpCpuOwnMaxFlowSolver<T>::solve(
    uint32_t max_coverage, bam_api::RegionApi& reg_api) {
    input_sequence_ = reg_api.get_paired_reads_aos();

    // This mapping is needed to create a cache-friendly data structure for
    // our implementation of push-relabel
    std::vector<std::size_t> sorted_indices(input_sequence_.reads.size());
    std::iota(sorted_indices.begin(), sorted_indices.end(), 0);
    std::sort(sorted_indices.begin(), sorted_indices.end(), [&](std::size_t a, std::size_t b) {
        return input_sequence_.reads[a].start_ind < input_sequence_.reads[b].start_ind;
    });

    T max_flow(input_sequence_.ref_genome_length + 3);

    std::size_t source_sink_edges_count = 0;
    // Pass the sorted indices vector to graph construction
    create_network_flow_graph(max_flow, input_sequence_, max_coverage, source_sink_edges_count, sorted_indices);

    const min_cost_flow_solver::Index source = input_sequence_.ref_genome_length + 1;
    const min_cost_flow_solver::Index sink = source + 1;
    const min_cost_flow_solver::Flow max_flow_val = max_flow.computeMaxFlow(source, sink);

    LOG_WITH_LEVEL(logging::INFO) << "Max flow problem result: " << max_flow_val;

    // Pass the sorted indices vector to result collection
    return obtain_sequence(max_flow, source_sink_edges_count, sorted_indices);
}

template <typename T>
void qmcp::QuasiMcpCpuOwnMaxFlowSolver<T>::create_network_flow_graph(
    T& max_flow, const bam_api::AOSPairedReads& sequence, unsigned int M,
    std::size_t& ss_count, const std::vector<std::size_t>& sorted_indices) {
    const min_cost_flow_solver::Index ref_len = sequence.ref_genome_length;
    const min_cost_flow_solver::Index source = ref_len + 1;
    const min_cost_flow_solver::Index sink = source + 1;

    const std::vector<int> demand = create_demand_function(sequence, M);

    for (min_cost_flow_solver::Index i = 0; i <= ref_len; ++i) {
        if (demand[i] > 0) {
            max_flow.addEdge(i, sink, static_cast<min_cost_flow_solver::Flow>(demand[i]));
            ss_count++;
        } else if (demand[i] < 0) {
            max_flow.addEdge(source, i, static_cast<min_cost_flow_solver::Flow>(-demand[i]));
            ss_count++;
        }
    }

    // Add read edges sequentially using the pre-sorted tracking vector
    for (std::size_t idx : sorted_indices) {
        const auto& read = sequence.reads[idx];
        max_flow.addEdge(read.start_ind, read.end_ind + 1, 1);
    }

    for (min_cost_flow_solver::Index i = 0; i < ref_len; ++i) {
        max_flow.addEdge(i + 1, i, std::numeric_limits<min_cost_flow_solver::Flow>::max());
    }
}

template <typename T>
std::vector<int> qmcp::QuasiMcpCpuOwnMaxFlowSolver<T>::create_b_function(
    const bam_api::AOSPairedReads& sequence, unsigned int M) {
    std::vector<int> b(sequence.ref_genome_length + 1, 0);

    for (std::size_t i = 0; i < sequence.reads.size(); ++i) {
        for (std::size_t j = sequence.reads[i].start_ind; j <= sequence.reads[i].end_ind; ++j) {
            ++b[j + 1];
        }
    }

    for (std::size_t i = 0; i < sequence.ref_genome_length + 1; ++i) {
        if (b[i] > static_cast<int>(M)) b[i] = static_cast<int>(M);
    }
    return b;
}

template <typename T>
std::vector<int> qmcp::QuasiMcpCpuOwnMaxFlowSolver<T>::create_demand_function(
    const bam_api::AOSPairedReads& sequence, unsigned int M) {
    std::vector<int> b = create_b_function(sequence, M);

    const int b_1 = b[1];
    for (std::size_t i = 1; i < sequence.ref_genome_length; ++i) {
        b[i] = b[i] - b[i + 1];
    }

    b[0] = -b_1;

    return b;
}

template <typename T>
std::unique_ptr<qmcp::Solution> qmcp::QuasiMcpCpuOwnMaxFlowSolver<T>::obtain_sequence(
    const T& max_flow,
    std::size_t ss_count, const std::vector<std::size_t>& sorted_indices) {
    auto reduced_reads = std::make_unique<Solution>();

    for (std::size_t internal_edge_idx = 0; internal_edge_idx < sorted_indices.size(); ++internal_edge_idx) {
        if (max_flow.getFlowVal(internal_edge_idx + ss_count) > 0) {
            reduced_reads->push_back(sorted_indices[internal_edge_idx]);
        }
    }

    return reduced_reads;
}
