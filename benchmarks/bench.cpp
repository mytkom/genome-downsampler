#include <benchmark/benchmark.h>

#include <cstdint>
#include <functional>
#include <map>
#include <memory>
#include <string>
#include <utility>
#include <vector>

#include "bam-api/aos_paired_reads.hpp"
#include "bam-api/bam_file_manager.hpp"
#include "bam-api/read.hpp"
#include "bam-api/region_api.hpp"
#include "logging/log.hpp"
#include "min-cost-flow-solver/push_relabel.hpp"
#include "min-cost-flow-solver/push_relabel_openmp.hpp"
#include "qmcp-solver/quasi_mcp_cpu_max_flow_solver.hpp"
#include "qmcp-solver/quasi_mcp_cpu_own_max_flow_solver.hpp"
#include "qmcp-solver/solver.hpp"
#include "reads_gen.hpp"

namespace bench {

constexpr uint32_t kSeed = 12345;
constexpr uint32_t kLargePairsCount = 1'000'000;
constexpr uint32_t kGenomeLength = 30'000;
constexpr uint32_t kReadLength = 150;


bam_api::AOSPairedReads get_small_aos_example() {
    bam_api::AOSPairedReads result;
    bam_api::ReadIndex id = 0;
    std::vector<bam_api::Read> reads = {
        {id++, 0, 2, 0, 3, 30, 10, true, false, false, false},
        {id++, 6, 9, 0, 4, 25, 15, false, false, false, false},
        {id++, 2, 4, 0, 3, 20, 12, true, false, false, false},
        {id++, 6, 8, 0, 3, 18, 14, false, false, false, false},
        {id++, 1, 3, 0, 3, 22, 11, true, false, false, false},
        {id++, 7, 10, 0, 4, 28, 13, false, false, false, false},
        {id++, 3, 6, 0, 4, 26, 16, true, false, false, false},
        {id++, 9, 10, 0, 2, 24, 17, false, false, false, false},
        {id++, 0, 4, 0, 5, 29, 18, true, false, false, false},
        {id++, 7, 9, 0, 3, 27, 19, false, false, false, false},
        {id++, 4, 6, 0, 3, 23, 20, true, false, false, false},
        {id++, 9, 10, 0, 2, 21, 21, false, false, false, false},
        {id++, 1, 4, 0, 4, 19, 22, true, false, false, false},
        {id++, 6, 8, 0, 3, 17, 23, false, false, false, false},
        {id++, 0, 2, 0, 3, 15, 24, true, false, false, false},
        {id++, 4, 6, 0, 3, 13, 25, false, false, false, false},
    };
    result.ref_genome_length = 11;
    for (auto& read : reads) {
        result.push_back(read);
    }
    return result;
}

bam_api::AOSPairedReads random_uniform(uint32_t reads_count) {
    std::mt19937 mt(kSeed);
    return reads_gen::rand_reads_uniform(mt, reads_count, kGenomeLength, kReadLength);
}

bam_api::AOSPairedReads random_with_func(const std::function<double(double)>& dist_func,
                                         uint32_t reads_count) {
    std::mt19937 mt(kSeed);
    return reads_gen::rand_reads(mt, reads_count, kGenomeLength, kReadLength, dist_func);
}

struct Scenario {
    const char* name;
    uint32_t max_coverage;
    std::function<bam_api::AOSPairedReads()> build;
};

const std::vector<Scenario>& scenarios() {
    static const std::vector<Scenario> kScenarios = {
        {"small_example", 4, [] { return get_small_aos_example(); }},
        {"random_uniform", 1000, [] { return random_uniform(kLargePairsCount); }},
        {"random_low_coverage_sides", 8000,
         [] { return random_with_func([](double x) { return x - x * x; }, kLargePairsCount); }},
        {"random_with_hole", 8000,
         [] {
             return random_with_func(
                 [](double x) {
                     if (x > 0.3684 && x < 0.6316) {
                         return 1000.0 * (x * x - x + 0.25) * (x * x - x + 0.25) + 0.2;
                     }
                     return 0.5;
                 },
                 kLargePairsCount);
         }},
        {"random_zero_coverage_sides", 8000,
         [] {
             return random_with_func([](double x) { return -10.0 * (x - 0.5) * (x - 0.5) + 1.0; },
                                     kLargePairsCount);
         }},
        {"real_1_2k", 2000,
          [] {
            bam_api::BamFileConfig config;
            bam_api::BamFileManager manager("benchmarks/real_examples/test.bam", config);
            auto regions = manager.get_regions();
            auto region = regions.begin();
            return region->second.get_paired_reads_aos();
        }},
        {"real_1_4k", 4000,
          [] {
            bam_api::BamFileConfig config;
            bam_api::BamFileManager manager("benchmarks/real_examples/test.bam", config);
            auto regions = manager.get_regions();
            auto region = regions.begin();
            return region->second.get_paired_reads_aos();
        }},
        {"real_1_8k", 8000,
          [] {
            bam_api::BamFileConfig config;
            bam_api::BamFileManager manager("benchmarks/real_examples/test.bam", config);
            auto regions = manager.get_regions();
            auto region = regions.begin();
            return region->second.get_paired_reads_aos();
        }},
    };
    return kScenarios;
}

class SolverRegistry {
   public:
    SolverRegistry() {
        emplace<qmcp::QuasiMcpCpuOwnMaxFlowSolver<
            min_cost_flow_solver::PushRelabel>>("quasi-mcp");
        emplace<qmcp::QuasiMcpCpuOwnMaxFlowSolver<
            min_cost_flow_solver::PushRelabelOpenMp>>("quasi-mcp-openmp");
        emplace<qmcp::QuasiMcpCpuMaxFlowSolver>("quasi-mcp-ortools");
    }
    const std::vector<std::string>& names() const { return names_; }
    qmcp::Solver& get(const std::string& name) const { return *solvers_.at(name); }

   private:
    template <typename SolverT>
    void emplace(const char* name) {
        names_.emplace_back(name);
        solvers_.emplace(name, std::make_unique<SolverT>());
    }
    std::vector<std::string> names_;
    std::map<std::string, std::unique_ptr<qmcp::Solver>> solvers_;
};
SolverRegistry& registry() {
    static SolverRegistry instance;
    return instance;
}

// --- benchmark helpers ---
void run_solve_benchmark(benchmark::State& state, const std::string& solver_name,
                         const Scenario& scenario) {
    auto input = scenario.build();
    bam_api::RegionApi reg_api(input);
    qmcp::Solver& solver = registry().get(solver_name);
    const double reads = static_cast<double>(input.get_reads_count());
    for (auto _ : state) {
        auto solution = solver.solve(scenario.max_coverage, reg_api);
        benchmark::DoNotOptimize(solution);
    }
    state.counters["reads"] = reads;
    state.counters["max_cov"] = static_cast<double>(scenario.max_coverage);
    state.counters["genome_len"] = static_cast<double>(input.ref_genome_length);
}

// Dimension 1 × 2: solver × scenario (fixed sizes from coverage_tester)
void register_fixed_scenario_benchmarks() {
    for (const auto& solver_name : registry().names()) {
        for (const auto& scenario : scenarios()) {
            const std::string bench_name = std::string(solver_name) + "/" + scenario.name;
            benchmark::RegisterBenchmark(bench_name.c_str(), [solver_name,
                                                              scenario](benchmark::State& state) {
                run_solve_benchmark(state, solver_name, scenario);
            })->Unit(benchmark::kMillisecond);
        }
    }
}

// Dimension 1 × 3: solver × pairs_count (uniform scenario only)
static void BM_UniformPairsScaling(benchmark::State& state) {
    const auto solver_idx = static_cast<size_t>(state.range(0));
    const uint32_t reads_count = static_cast<uint32_t>(state.range(1));
    constexpr uint32_t kMaxCoverage = 1000;
    const auto& solver_name = registry().names().at(solver_idx);
    Scenario scenario{"random_uniform_scaled", kMaxCoverage,
                      [reads_count] { return random_uniform(reads_count); }};
    run_solve_benchmark(state, solver_name, scenario);
}

static void register_uniform_scaling_args(benchmark::internal::Benchmark* b) {
    for (int64_t solver_idx = 0; solver_idx < static_cast<int64_t>(registry().names().size());
         ++solver_idx) {
        for (int64_t reads : {10'000, 100'000, 1'000'000}) {
            b->Args({solver_idx, reads});
        }
    }
}

struct AutoRegister {
    AutoRegister() {
        SET_LOG_LEVEL(logging::ERROR);
        register_fixed_scenario_benchmarks();
        BENCHMARK(BM_UniformPairsScaling)
            ->Apply(register_uniform_scaling_args)
            ->ArgNames({"solver_idx", "reads"})
            ->Unit(benchmark::kMillisecond);
    }
};

const AutoRegister kAutoRegister{};

}  // namespace bench
