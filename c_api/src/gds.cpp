#include "gds.h"

#include <algorithm>
#include <cstring>
#include <filesystem>
#include <map>
#include <memory>
#include <string>
#include <unordered_set>

#include "bam-api/bam_file_config_builder.hpp"
#include "bam-api/bam_file_manager.hpp"
#include "logging/log.hpp"
#include "qmcp-solver/solver.hpp"
#include "solver_manager.hpp"

// Cpp helpers
namespace {

std::string g_last_error;

void set_error(const std::string& message) { g_last_error = message; }

bool is_empty_path(const char* path) { return path == nullptr || path[0] == '\0'; }

std::filesystem::path to_path(const char* path) { return std::filesystem::path(path); }

}  // namespace

extern "C" void gds_config_init(GdsConfig* config) {
    if (config == nullptr) {
        return;
    }

    // allocate config object Cppish
    std::memset(config, 0, sizeof(GdsConfig));

    // set defaults :>
    config->solver_name = "quasi-mcp";
    config->hts_thread_count = 2;
    config->min_mapq = 30;
    config->min_seq_length = 90;
    config->min_alignment = 0.5f;
}

extern "C" const char* gds_last_error(void) { return g_last_error.c_str(); }

extern "C" int gds_downsample(const GdsConfig* config, const char* input_path,
                              const char* output_path) {
    g_last_error.clear();

    if (config == nullptr || input_path == nullptr || output_path == nullptr) {
        set_error("config, input_path, and output_path must be non-null");
        return 1;
    }

    if (config->max_coverage == 0) {
        set_error("max_coverage must be greater than 0");
        return 1;
    }

    // C API, cannot raise exception, must be catched
    try {
        SET_LOG_LEVEL(config->verbose ? logging::DEBUG : logging::INFO);

        // code like in src/app.cpp
        SolverManager solver_manager;
        const char* solver_name =
            (config->solver_name == nullptr || config->solver_name[0] == '\0')
                ? "quasi-mcp"
                : config->solver_name;

        if (!solver_manager.contains(solver_name)) {
            set_error(std::string("unknown solver: ") + solver_name);
            return 1;
        }

        bam_api::BamFileConfigBuilder config_builder;
        config_builder.add_hts_thread_count(config->hts_thread_count == 0 ? 2
                                                                        : config->hts_thread_count);
        config_builder.add_min_mapq(config->min_mapq);
        config_builder.add_min_seq_length(config->min_seq_length);
        config_builder.add_amp_overflow(config->amp_overflow);
        config_builder.add_min_alignment(config->min_alignment);

        if (!is_empty_path(config->bed_path)) {
            qmcp::Solver& solver = solver_manager.get(solver_name);
            if (solver.uses_quality_of_reads()) {
                config_builder.add_amplicon_filtering(
                    bam_api::AmpliconBehaviour::GRADE, to_path(config->bed_path),
                    is_empty_path(config->tsv_path) ? std::filesystem::path()
                                                    : to_path(config->tsv_path));
            } else {
                config_builder.add_amplicon_filtering(
                    bam_api::AmpliconBehaviour::FILTER, to_path(config->bed_path),
                    is_empty_path(config->tsv_path) ? std::filesystem::path()
                                                    : to_path(config->tsv_path));
            }
        }

        bam_api::BamFileManager file_manager(to_path(input_path), config_builder.build());
        std::map<std::string, std::vector<bam_api::ReadIndex>> complete_solution;

        for (auto& [region, region_api] : file_manager.get_regions()) {
            LOG_WITH_LEVEL(logging::INFO) << "processing " << region;
            std::unordered_set<bam_api::ReadIndex> pseudo_reads_ids = region_api.add_pseudo_reads();
            std::unique_ptr<qmcp::Solution> solution =
                solver_manager.get(solver_name).solve(config->max_coverage, region_api);
            solution->erase(std::remove_if(solution->begin(), solution->end(),
                                             [&](bam_api::ReadIndex id) {
                                                 return pseudo_reads_ids.count(id) != 0;
                                             }),
                            solution->end());
            complete_solution[region] = region_api.find_pairs(*solution);
        }

        file_manager.write_paired_reads(to_path(output_path), complete_solution);

        if (!is_empty_path(config->preprocessing_out_path)) {
            file_manager.write_bam_api_filtered_out_reads(to_path(config->preprocessing_out_path));
        }
    } catch (const std::exception& e) {
        set_error(e.what());
        return 1;
    } catch (...) {
        set_error("unknown error");
        return 1;
    }

    return 0;
}
