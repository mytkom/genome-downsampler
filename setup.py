import os
from pathlib import Path

from setuptools import Extension, setup, find_packages

ROOT = Path(__file__).resolve().parents[0]
GDS_INCLUDE = ROOT / "c_api" / "include"
GDS_LIB_DIR = Path(os.environ.get("GDS_LIBRARY_DIR", ROOT / "build/gcc-x64-release/c_api"))

gds_extension = Extension(
    "genome_downsampler._gds",
    sources=["genome_downsampler/_gdsmodule.c"],
    include_dirs=[str(GDS_INCLUDE)],
    libraries=["gds_c"],
    library_dirs=[str(GDS_LIB_DIR)],
    runtime_library_dirs=[str(GDS_LIB_DIR.resolve())],
)

setup(
    name="genome-downsampler",
    version="0.1.0",
    description="Python bindings for the genome-downsampler C API",
    packages=find_packages(),
    license="MIT",
    include_package_data=True,
    author="Marek Mytkowski, Borys Kurdek, Michał Okurowski",
    author_email="marek.mytkowski.mm@gmail.com",
    maintainer="Marek Mytkowski",
    ext_modules=[gds_extension],
    python_requires=">=3.9",
)
