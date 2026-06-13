"""Python bindings for the genome-downsampler C API.

OpenMP (for solver ``quasi-mcp-openmp``) is provided by ``libgds_c``. Control it with
environment variables before calling :func:`downsample`, e.g. ``OMP_NUM_THREADS``.
"""

from . import _gds
from .config import Config

__all__ = ["Config", "downsample"]


def downsample(config: Config, input_path: str, output_path: str) -> None:
    """Run downsampling on a BAM file.

    Args:
        config: Preprocessing and solver settings.
        input_path: Path to input ``.bam`` file.
        output_path: Path to output ``.bam`` file.

    Raises:
        RuntimeError: If the native call fails.
    """
    _gds.downsample(
        input_path,
        output_path,
        config.max_coverage,
        solver_name=config.solver_name,
        hts_thread_count=config.hts_thread_count,
        min_mapq=config.min_mapq,
        min_seq_length=config.min_seq_length,
        amp_overflow=config.amp_overflow,
        min_alignment=config.min_alignment,
        bed_path=config.bed_path,
        tsv_path=config.tsv_path,
        preprocessing_out_path=config.preprocessing_out_path,
        verbose=config.verbose,
    )
