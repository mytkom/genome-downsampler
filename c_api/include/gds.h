#pragma once

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

#define GDS_API __attribute__((visibility("default")))

typedef struct GdsConfig {
    uint32_t max_coverage;
    const char* solver_name;

    uint32_t hts_thread_count;
    uint32_t min_mapq;
    uint32_t min_seq_length;
    uint32_t amp_overflow;
    float min_alignment;

    const char* bed_path;
    const char* tsv_path;
    const char* preprocessing_out_path;

    int verbose;
} GdsConfig;

GDS_API void gds_config_init(GdsConfig* config);

GDS_API int gds_downsample(const GdsConfig* config, const char* input_path,
                           const char* output_path);

GDS_API const char* gds_last_error(void);

#ifdef __cplusplus
}
#endif
