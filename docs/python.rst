Python bindings
===============

The ``genome_downsampler`` package wraps ``libgds_c``. It uses the same downsampling logic as the CLI.

A Bioconda/PyPI package is planned. Packaging work is still in progress.

Build
-----

You need the same dependencies as the C++ app, plus Python 3.9+ development headers.

.. code-block:: sh

   cmake --preset gcc-x64-release
   cmake --build build/gcc-x64-release --target gds_c

   GDS_LIBRARY_DIR=build/gcc-x64-release/c_api pip install -e .

``GDS_LIBRARY_DIR`` must point to the directory containing ``libgds_c.so``.

Use
---

.. code-block:: python

   import genome_downsampler as gds

   config = gds.Config(
       max_coverage=1000,
       solver_name="quasi-mcp",
   )

   gds.downsample(config, "input.bam", "output.bam")

``Config`` fields match ``GdsConfig`` in the C API. See ``genome_downsampler/config.py`` for defaults.

OpenMP solvers (for example ``quasi-mcp-openmp``) use the runtime inside ``libgds_c``. You can set ``OMP_NUM_THREADS`` before calling ``downsample``.

Tests
-----

.. code-block:: sh

   python3 tests.py -a quasi-mcp quasi-mcp-openmp

Uses the committed fixture ``tests/fixtures/sample.bam``.

Without ``-a``, all known solvers are tested (same idea as ``genome-downsampler test -a``).
