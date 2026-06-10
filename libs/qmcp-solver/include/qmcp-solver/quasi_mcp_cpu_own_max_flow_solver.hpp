#ifndef QMCP_QUASI_MCP_CPU_OWN_MAX_FLOW_SOLVER_HPP
#define QMCP_QUASI_MCP_CPU_OWN_MAX_FLOW_SOLVER_HPP
#include <memory>
#include <vector>

#include "bam-api/region_api.hpp"
#include "solver.hpp"

namespace qmcp {

template <typename T>
class QuasiMcpCpuOwnMaxFlowSolver : public Solver {
   public:
    std::unique_ptr<Solution> solve(uint32_t max_coverage, bam_api::RegionApi& reg_api) override;
    bool uses_quality_of_reads() override { return false; }

   private:
    static void create_network_flow_graph(T& max_flow, const bam_api::AOSPairedReads& sequence,
                                          unsigned int M, std::size_t& ss_count,
                                          const std::vector<std::size_t>& sorted_indices);
    static std::vector<int> create_demand_function(const bam_api::AOSPairedReads& sequence,
                                                   unsigned int M);
    static std::unique_ptr<Solution> obtain_sequence(
        const T& max_flow, std::size_t ss_count, const std::vector<std::size_t>& sorted_indices);
    static std::vector<int> create_b_function(const bam_api::AOSPairedReads& sequence,
                                              unsigned int M);

    bam_api::AOSPairedReads input_sequence_;
};

}  // namespace qmcp

#endif
