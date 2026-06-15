Introduction
============

Genome Downsampler reduces coverage in aligned paired-end sequencing data while keeping pairs together and respecting a per-base coverage limit.

It reads a BAM file, runs a selected solver on each reference region, and writes a downsampled BAM file.

Supported interfaces:

* C++ command-line application
* C++ ``qmcp-solver`` library - solvers for BAM API objects
* C++ ``bam-api`` library - cpp wrapper around HTSLib with filtering and efficient reading and writing BAM/SAM files (uses minimal RAM, loads only required data).
* C shared library (``libgds_c``)
* Python package (``genome_downsampler``) - it runs the same pipeline as C++ CLI, but can be composed into other preprocessing, Python-based scripts. It in the future is planned to support ``pysam`` objects and be published on PyPI/bioconda.

Platform support is GNU/Linux only and it is tested on Ubuntu 22.04 programatically.
