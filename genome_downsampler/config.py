"""Configuration for genome downsampling."""

from dataclasses import dataclass
from typing import Optional


@dataclass
class Config:
    """BAM preprocessing and solver settings for Genome Downsampler.

    Attributes:
        max_coverage: Maximum coverage per reference genome base pair (index). Required, must be > 0.
        solver_name: Solver id (e.g. ``"quasi-mcp"``, ``"quasi-mcp-ortools"``).
            Default: ``"quasi-mcp"``.
        hts_thread_count: Thread count for htslib read/write. Default: ``2``.
        min_mapq: Reads with MAPQ below this value are filtered out. Default: ``30``.
        min_seq_length: Reads shorter than this are filtered out. Default: ``90``.
        amp_overflow: Bp a read may extend outside an amplicon and still count as inside.
            Default: ``0``.
        min_alignment: Minimum alignment ratio (AS / sequence length). Default: ``0.5``.
        bed_path: Optional ``.bed`` amplicon bounds file. Enables amplicon filtering/grading.
        tsv_path: Optional ``.tsv`` pairing amplicons from the bed file. Used with ``bed_path``.
        preprocessing_out_path: Optional ``.bam`` path for reads filtered during preprocessing.
        verbose: Enable debug logging when ``True``. Default: ``False``.
    """

    max_coverage: int
    solver_name: str = "quasi-mcp"
    hts_thread_count: int = 2
    min_mapq: int = 30
    min_seq_length: int = 90
    amp_overflow: int = 0
    min_alignment: float = 0.5
    bed_path: Optional[str] = None
    tsv_path: Optional[str] = None
    preprocessing_out_path: Optional[str] = None
    verbose: bool = False
