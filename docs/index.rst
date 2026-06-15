Genome Downsampler
==================

Coverage-constrained downsampling of paired sequencing reads in BAM files.

Examples of downsampling. Function of coverage of each reference genome index.

.. _real-data:

.. figure:: images/quasi_mcp_vs_mcp_coverage_comparation_real_2000.png
   :alt: Real world results
   :align: center
   :width: 90%

   Results of downsampling on a real-world SARS-CoV-2 paired-end RNA dataset.

.. _synthetic-hole:

.. figure:: images/quasi_mcp_vs_mcp_coverage_comparation_with_hole.png
   :alt: Synthetic data with a coverage hole
   :align: center
   :width: 90%

   Results on synthetic data with a large coverage gap in the middle, showing typical downsampling behaviour.

Intrigued? See :doc:`introduction` to read more!

.. toctree::
   :maxdepth: 2
   :caption: Contents

   introduction
   theory
   cpp_cli
   c_api
   python
   benchmarks
   development
