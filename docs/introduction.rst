Introduction
============

Why downsampling exists
-----------------------

Genome studies often look for **mutations** — differences from a reference genome. Sequencing does not return one long genome string. It returns **many short reads**. Reads can be mis-mapped, overlap, and cover the reference **unevenly**.

Researchers therefore collect extra data and later **downsample**: remove lower-value reads while keeping enough coverage at each reference position. That is the problem this project solves.

Extra constraints in practice:

* Reads are often **paired** — you keep or drop the **whole pair**, never one mate alone.
* **Amplicons** (intervals on the reference) may be given. Pairs with both mates in one amplicon are preferred over pairs split across amplicons.
* Read **MAPQ** (mapping quality) guides which reads to prefer when several downsamples are possible.

What this software does
-----------------------

Genome Downsampler reads an aligned BAM file, runs a solver on each reference region, and writes a smaller BAM that still meets a per-base coverage target.

It does not look at ACGT letters during optimization — only **reference indices**, read intervals, MAPQ, and optional amplicon metadata.

Supported interfaces:

* C++ command-line application
* C++ ``qmcp-solver`` library — solvers for BAM API objects
* C++ ``bam-api`` library — wrapper around HTSlib with filtering and efficient BAM I/O (minimal RAM, loads only what is needed)
* C shared library (``libgds_c``)
* Python package (``genome_downsampler``) — same pipeline as the CLI for scripting; PyPI/bioconda packaging is planned

Platform support is GNU/Linux (tested on Ubuntu 22.04 in CI).

Motivating dataset
------------------

Our main test case is **paired-end RNA sequencing for SARS-CoV-2**:

* reference length :math:`N \approx 3 \times 10^4` bp
* read length :math:`n \approx 150` bp
* on the order of :math:`10^7` read **pairs** (:math:`2 \times 10^7` mates)

So :math:`M \gg N`. Good algorithms should limit how strongly runtime and memory grow with the number of input reads :math:`M`.

.. _real-data-2:

.. figure:: images/quasi_mcp_vs_mcp_coverage_comparation_real_2000.png
   :alt: Real world results
   :align: center
   :width: 90%

   Results of downsampling on a real-world SARS-CoV-2 paired-end RNA dataset.


Goal in one sentence
--------------------

Find the **smallest** set of paired reads such that every reference index has coverage at least :math:`m`, preferring higher MAPQ (and, when enabled, same-amplicon pairs) — without reducing coverage below :math:`m` anywhere that already met it before downsampling.

See :doc:`theory` for the formal definitions and the flow-network formulation.
