Benchmarks
==========

During development we gained a desire to remove OR-Tools from our code (heavy, external dependency). Thus, we've implemented max flow algorithm, namely push-relabel with global relabel heuristics (the very same as in OR-Tools, though implemented differently). To improve it even further we've added a parallel implementation using OpenMP, though it is just a parallelised version of original code, no algorithm-wise changes has been made (maybe apart from batching).

Currently only quasi-MCP method has been implemented by us and we are hoping to get performance on par or even better than OR-Tools. Below are current benchmarks performed on T495 laptop with 24GB of RAM (AMD Ryzen 3500u - 8 threads).

Solvers compared
----------------

We benchmark three quasi-MCP backends that differ only in the max-flow engine:

.. list-table::
   :header-rows: 1

   * - Name
     - Flow backend
   * - ``quasi-mcp``
     - ``PushRelabel`` — single-threaded push-relabel
   * - ``quasi-mcp-openmp``
     - ``PushRelabelOpenMp`` — same algorithm, OpenMP parallel discharge
   * - ``quasi-mcp-ortools``
     - OR-Tools max-flow API (``QuasiMcpCpuMaxFlowSolver``)

How we measure
--------------

Executable: ``solver-bench`` (``benchmarks/bench.cpp``). For each case we build a ``RegionApi`` fixture once, then time only ``solver.solve(max_coverage, region)`` in a Google Benchmark loop.

.. code-block:: sh

   cmake --preset gcc-x64-release -DWITH_BENCHMARK=ON
   cmake --build build/gcc-x64-release --target solver-bench
   ./build/gcc-x64-release/benchmarks/solver-bench --benchmark_min_time=5s

Tables below use **CPU time** in milliseconds. Build: ``gcc-x64-release``, ``--benchmark_min_time=5s``.

Test sample generation
--------------------

Synthetic inputs use the same generators as ``coverage_tester.cpp`` / ``reads_gen``. Shared constants (unless noted):

.. math::

   N &= 1{,}000{,}000 \quad \text{(read pairs)} \\
   L &= 30{,}000 \quad \text{(reference length, bp)} \\
   \ell &= 150 \quad \text{(fixed read length, bp)} \\
   \text{seed} &= 12{,}345 \quad \text{(}\texttt{std::mt19937}\text{)}

Each pair yields two mates of length :math:`\ell` on :math:`[0, L-1]`. After drawing starts, mates are ordered and separated so intervals do not overlap incorrectly (see code in ``reads_gen``).

small_example
~~~~~~~~~~~~~

Fixed hand-written instance: 8 pairs (16 reads), :math:`L = 11`, ``max_coverage`` :math:`m = 4`. Used as a sanity / overhead check.

random_uniform
~~~~~~~~~~~~~~

``reads_gen::rand_reads_uniform``. Mate 1 start :math:`s_1 \sim \mathrm{Uniform}\{0, \ldots, L - 2\ell\}`, mate 2 start :math:`s_2 \sim \mathrm{Uniform}\{0, \ldots, L - \ell\}`; then sorted and adjusted so :math:`s_2 \ge s_1 + \ell` when needed. Benchmark uses :math:`m = 1000`.

random_low_coverage_sides
~~~~~~~~~~~~~~~~~~~~~~~~~

``reads_gen::rand_reads`` with weight function on normalised coordinate :math:`x \in [0,1]`:

.. math::

   f(x) = x - x^2 = x(1-x)

For each valid start index :math:`i \in \{0,\ldots,n-1\}` with :math:`n = L - \ell + 1` and :math:`x_i = i/(n-1)`, weight :math:`w_i = \max(f(x_i), 0)`, probability :math:`p_i = w_i / \sum_j w_j`. Each mate start is drawn from :math:`p` independently (then pairing rules apply). :math:`m = 8000`.

This peaks at :math:`x = 0.5` and goes to zero at both ends — lower expected coverage near the flanks.

random_with_hole
~~~~~~~~~~~~~~~~

Piecewise :math:`f`:

.. math::

   f(x) = \begin{cases}
     1000\,(x^2 - x + 0.25)^2 + 0.2 & 0.3684 < x < 0.6316 \\
     0.5 & \text{otherwise}
   \end{cases}

Note :math:`x^2 - x + 0.25 = (x - \tfrac{1}{2})^2`, so the centre is a narrow high-coverage bump; flanks are flat at 0.5. Same :math:`p_i` construction as above. :math:`m = 8000`.

random_zero_coverage_sides
~~~~~~~~~~~~~~~~~~~~~~~~~~

Inverted parabola:

.. math::

   f(x) = -10\,(x - 0.5)^2 + 1

So :math:`f(0.5) = 1`, :math:`f(0) = f(1) = 0` (clamped to zero before normalisation). High coverage in the middle, none at the ends. :math:`m = 8000`.

real_1_2k / real_1_4k / real_1_8k
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

No synthetic generator: paired reads are loaded from ``benchmarks/real_examples/test.bam`` (~2.24M pairs, :math:`L \approx 29{,}903`). Only ``max_coverage`` changes: :math:`m \in \{2000, 4000, 8000\}`.

Uniform scaling
~~~~~~~~~~~~~~~

Same as ``random_uniform`` but :math:`N \in \{10{,}000,\; 100{,}000,\; 1{,}000{,}000\}`, fixed :math:`m = 1000`. Registered as ``BM_UniformPairsScaling`` with ``solver_idx`` 0 / 1 / 2 matching the three solvers above.

Fixed scenarios
---------------

.. list-table::
   :header-rows: 1
   :widths: 28 18 18 18

   * - Scenario
     - quasi-mcp
     - quasi-mcp-openmp
     - quasi-mcp-ortools
   * - small_example
     - 0.002
     - 0.070
     - 0.005
   * - random_uniform
     - 766
     - 746
     - **324**
   * - random_low_coverage_sides
     - 1204
     - 1093
     - **739**
   * - random_with_hole
     - 1362
     - 1117
     - **819**
   * - random_zero_coverage_sides
     - 1000
     - 909
     - **519**
   * - real_1_2k
     - 6418
     - 2008
     - **1690**
   * - real_1_4k
     - 6355
     - 2989
     - **2748**
   * - real_1_8k
     - 6166
     - 3706
     - **3217**

Uniform scaling (CPU ms)
------------------------

:math:`N` read pairs, uniform random placement, :math:`m = 1000`.

.. list-table::
   :header-rows: 1
   :widths: 22 18 18 18

   * - :math:`N`
     - quasi-mcp
     - quasi-mcp-openmp
     - quasi-mcp-ortools
   * - 10k
     - **8.79**
     - 27.4
     - 19.0
   * - 100k
     - 535
     - 971
     - **251**
   * - 1M
     - 784
     - 772
     - **322**

What we see so far
------------------

OR-Tools is still faster on every large synthetic and real case here — often about 2× on 1M-pair uniform input. Our single-thread Push-Relabel only wins on ``small_example`` and at :math:`N = 10{,}000`, where fixed overhead dominates.

OpenMP helps most on real BAM data at moderate :math:`m` (e.g. ``real_1_2k``: 6418 ms down to 2008 ms CPU, vs OR-Tools 1690 ms). At ``real_1_8k`` it narrows the gap but does not beat OR-Tools in this run. At :math:`N = 1{,}000{,}000` uniform, single-thread and OpenMP are both ~770–780 ms; OR-Tools stays ~322 ms.

So today: ``quasi-mcp-ortools`` for raw speed on big inputs; ``quasi-mcp-openmp`` if we want an in-house backend on real coverage shapes; ``quasi-mcp`` for tiny regions or when avoiding OR-Tools matters more than throughput.

.. note::
   Compared on a single machine with a limited number of scenarios.
