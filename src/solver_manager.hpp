#ifndef SOLVER_MANAGER_HPP
#define SOLVER_MANAGER_HPP

#include <map>
#include <memory>
#include <string>
#include <vector>

#include "helpers.hpp"
#include "min-cost-flow-solver/push_relabel.hpp"
#include "min-cost-flow-solver/push_relabel_openmp.hpp"
#include "min-cost-flow-solver/push_relabel_vector_simple_heuristics.hpp"
#include "min-cost-flow-solver/push_relabel_simple_heuristics.hpp"
#include "qmcp-solver/mcp_cpu_cost_scaling_solver.hpp"
#include "qmcp-solver/qmcp_cpu_cost_scaling_solver.hpp"
#include "qmcp-solver/quasi_mcp_cpu_max_flow_solver.hpp"
#include "qmcp-solver/quasi_mcp_cpu_own_max_flow_solver.hpp"
#include "qmcp-solver/quasi_mcp_cuda_max_flow_solver.hpp"
#include "qmcp-solver/solver.hpp"
#include "qmcp-solver/test_solver.hpp"

class SolverManager {
   public:
    SolverManager() {
        solvers_map_.emplace("test-solver", std::make_unique<qmcp::TestSolver>());
        solvers_map_.emplace("quasi-mcp",
                             std::make_unique<qmcp::QuasiMcpCpuOwnMaxFlowSolver<
                                 min_cost_flow_solver::PushRelabel>>());
        solvers_map_.emplace("quasi-mcp-openmp",
                             std::make_unique<qmcp::QuasiMcpCpuOwnMaxFlowSolver<
                                 min_cost_flow_solver::PushRelabelOpenMp>>());
        // solvers_map_.emplace("quasi-mcp-struct",
        //                      std::make_unique<qmcp::QuasiMcpCpuOwnMaxFlowSolver<
        //                          min_cost_flow_solver::PushRelabelSimpleHeuristics>>());
        // solvers_map_.emplace("quasi-mcp-vector",
        //                      std::make_unique<qmcp::QuasiMcpCpuOwnMaxFlowSolver<
        //                          min_cost_flow_solver::PushRelabelVectorSimpleHeuristics>>());
        solvers_map_.emplace("quasi-mcp-ortools",
                             std::make_unique<qmcp::QuasiMcpCpuMaxFlowSolver>());
        solvers_map_.emplace("mcp-ortools", std::make_unique<qmcp::McpCpuCostScalingSolver>());
        solvers_map_.emplace("qmcp-ortools", std::make_unique<qmcp::QmcpCpuCostScalingSolver>());
#ifdef CUDA_ENABLED
        solvers_map_.emplace("quasi-mcp-cuda", std::make_unique<qmcp::QuasiMcpCudaMaxFlowSolver>());
#endif

        algorithms_names_ = helpers::get_names_from_map(solvers_map_);
    }

    qmcp::Solver& get(const std::string& solver_name) const {
        return *solvers_map_.at(solver_name);
    }

    bool contains(const std::string& solver_name) {
        return solvers_map_.find(solver_name) != solvers_map_.end();
    }

    const std::vector<std::string>& get_names() const { return algorithms_names_; }

   private:
    std::map<std::string, std::unique_ptr<qmcp::Solver>> solvers_map_;
    std::vector<std::string> algorithms_names_;
};

#endif
