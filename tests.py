#!/usr/bin/env python3
"""Integration tests for genome_downsampler Python bindings."""

from __future__ import annotations

import argparse
import sys
import tempfile
import unittest
from pathlib import Path

import genome_downsampler as gds

ROOT = Path(__file__).resolve().parent
INPUT_BAM = ROOT / "tests/fixtures/sample.bam"
MAX_COVERAGE = 1000

KNOWN_SOLVERS = [
    "quasi-mcp",
    "quasi-mcp-openmp",
    "quasi-mcp-ortools",
    "mcp-ortools",
    "qmcp-ortools",
]


def parse_args(argv: list[str] | None = None) -> tuple[argparse.Namespace, list[str]]:
    parser = argparse.ArgumentParser(
        description="Run genome-downsampler integration tests.",
        formatter_class=argparse.ArgumentDefaultsHelpFormatter,
    )
    parser.add_argument(
        "-a",
        "--algorithms",
        nargs="+",
        metavar="SOLVER",
        choices=KNOWN_SOLVERS,
        help="Solvers to test. Default: all known solvers.",
    )
    return parser.parse_known_args(argv)


class DownsampleIntegrationTests(unittest.TestCase):
    algorithms: list[str] = KNOWN_SOLVERS

    @classmethod
    def setUpClass(cls) -> None:
        if not INPUT_BAM.is_file():
            raise unittest.SkipTest(f"missing fixture: {INPUT_BAM}")

    def test_downsample(self) -> None:
        for solver_name in self.algorithms:
            with self.subTest(solver=solver_name):
                with tempfile.TemporaryDirectory() as tmp_dir:
                    output_bam = Path(tmp_dir) / f"{solver_name}.bam"
                    config = gds.Config(max_coverage=MAX_COVERAGE, solver_name=solver_name)
                    gds.downsample(config, str(INPUT_BAM), str(output_bam))
                    self.assertTrue(output_bam.is_file())
                    self.assertGreater(output_bam.stat().st_size, 0)


def main(argv: list[str] | None = None) -> int:
    argv = list(sys.argv if argv is None else argv)
    args, unittest_argv = parse_args(argv)
    if not unittest_argv:
        unittest_argv = [argv[0]]
    DownsampleIntegrationTests.algorithms = args.algorithms or KNOWN_SOLVERS
    return unittest.main(argv=unittest_argv)


if __name__ == "__main__":
    sys.exit(main())
