C API
=====

The C API is provided by ``libgds_c`` (build with ``-DWITH_C_API=ON``).

Header: ``c_api/include/gds.h``

Build
-----

.. code-block:: sh

   cmake --preset gcc-x64-release -DWITH_C_API=ON
   cmake --build build/gcc-x64-release --target gds_c

The shared library is written to ``build/gcc-x64-release/c_api/libgds_c.so``.

``GdsConfig``
-------------

Configuration for BAM preprocessing and the solver. Initialize with ``gds_config_init`` before setting fields.

.. list-table::
   :header-rows: 1

   * - Field
     - Description
   * - ``max_coverage``
     - Maximum coverage per reference base. Required.
   * - ``solver_name``
     - Solver id. Default after init: ``"quasi-mcp"``.
   * - ``hts_thread_count``
     - htslib threads. Default: ``2``.
   * - ``min_mapq``
     - MAPQ filter. Default: ``30``.
   * - ``min_seq_length``
     - Length filter. Default: ``90``.
   * - ``amp_overflow``
     - Amplicon boundary slack (bp). Default: ``0``.
   * - ``min_alignment``
     - Minimum AS / length ratio. Default: ``0.5``.
   * - ``bed_path``
     - Optional ``.bed`` path, or ``NULL``.
   * - ``tsv_path``
     - Optional ``.tsv`` path, or ``NULL``.
   * - ``preprocessing_out_path``
     - Optional filtered-reads BAM path, or ``NULL``.
   * - ``verbose``
     - Non-zero enables debug logging.

Functions
---------

``void gds_config_init(GdsConfig* config)``
   Zero the struct and apply defaults.

``int gds_downsample(const GdsConfig* config, const char* input_path, const char* output_path)``
   Run downsampling. Returns ``0`` on success, ``1`` on error.

``const char* gds_last_error(void)``
   Error message after a failed call.

Example
-------

.. code-block:: c

   #include "gds.h"
   #include <stdio.h>

   int main(void) {
       GdsConfig cfg;
       gds_config_init(&cfg);
       cfg.max_coverage = 1000;
       cfg.solver_name = "quasi-mcp";

       if (gds_downsample(&cfg, "input.bam", "output.bam") != 0) {
           fprintf(stderr, "%s\n", gds_last_error());
           return 1;
       }
       return 0;
   }

Link with ``-lgds_c`` and add the library directory to the linker search path.
