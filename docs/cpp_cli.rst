C++ CLI
=======

Build the ``genome-downsampler`` executable from source (see :doc:`development`), then run:

.. code-block:: sh

   genome-downsampler INPUT.bam MAX_COVERAGE [options]

Positional arguments
--------------------

``INPUT_FILEPATH``
   Path to the input ``.bam`` file. Required.

``MAX_COVERAGE``
   Maximum coverage per reference base pair. Required. Must be greater than 0.

Optional arguments
------------------

``-h``, ``--help``
   Print help and exit.

``-o``, ``--output`` ``PATH``
   Output ``.bam`` path. Default: ``output.bam`` next to the input file.

``-a``, ``--algorithm`` ``NAME``
   Solver to use. Default: ``quasi-mcp``.

   Examples: ``quasi-mcp``, ``quasi-mcp-openmp``, ``quasi-mcp-ortools``, ``quasi-mcp-cuda``, ``mcp-ortools``, ``qmcp-ortools``.

``-b``, ``--bed`` ``FILE``
   ``.bed`` file with amplicon bounds.

``-t``, ``--tsv`` ``FILE``
   ``.tsv`` file describing primer pairings from the bed file.

``-p``, ``--preprocessing-out`` ``PATH``
   Optional ``.bam`` for reads filtered during preprocessing.

``-l``, ``--min-length`` ``UINT``
   Minimum read length. Default: ``90``.

``-q``, ``--min-mapq`` ``UINT``
   Minimum MAPQ. Default: ``30``.

``-f``, ``--amp-overflow`` ``UINT``
   Bp a read may extend outside an amplicon. Default: ``0``.

``-g``, ``--min-alignment`` ``FLOAT``
   Minimum AS / sequence length ratio in ``[0, 1]``. Default: ``0.5``.

``-@``, ``--threads`` ``UINT``
   htslib thread count. Default: ``2``.

``-v``, ``--verbose``
   Enable debug logging.

Test subcommand
---------------

With ``WITH_TESTS=ON`` at configure time:

.. code-block:: sh

   genome-downsampler test [-a SOLVER ...] [-t TEST ...] [-o OUTPUT_DIR]

``-a`` selects solvers to test. With no ``-a``, all solvers are run.

Examples
--------

Basic:

.. code-block:: sh

   genome-downsampler /data/input.bam 100

With options:

.. code-block:: sh

   genome-downsampler /data/input.bam 100 \
     -a quasi-mcp-openmp \
     -o /data/output.bam \
     -v

Docker
------

.. code-block:: sh

   docker build -t genome-downsampler .
   docker run -it -v /home/user/data:/data genome-downsampler /data/sample.bam 100 -o /data/output.bam
