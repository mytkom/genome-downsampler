Development
===========

Contributing
------------

Branch naming:

.. code-block:: text

   (feature|bug)/{issue_number}(-{feature_name})?

Examples:

* ``feature/4-add-min-cost-max-flow-library``
* ``bug/7``

Project layout
--------------

Libraries live under ``libs/``. The main application is under ``src/``. CMake presets are in ``CMakePresets.json``.

Toolchain
---------

* C++17
* **gcc/g++** >= 11 (primary compiler; CUDA 12.4 support)
* **clang/clang++** >= 14 (clangd / lint; CUDA support is limited)
* **CMake** >= 3.21
* **CUDA** 12.4 (optional, for GPU solvers)

Build
-----

Default release build:

.. code-block:: sh

   cmake --preset gcc-x64-release
   cmake --build --preset gcc-x64-release

With tests (enables ``test`` subcommand):

.. code-block:: sh

   cmake --preset gcc-x64-debug -DWITH_TESTS=ON
   cmake --build --preset gcc-x64-debug

CMake options:

* ``WITH_CUDA`` — CUDA solvers
* ``WITH_TESTS`` — ``test`` subcommand
* ``WITH_C_API`` — ``libgds_c``
* ``WITH_BENCHMARK`` — Google Benchmark executable

Linting
-------

* **clang-format** >= 14 — formatting
* **clang-tidy** >= 14 — static analysis (``.clang-tidy``)
* **python3** — ``./scripts/run-clang-tidy.sh``

Dependencies
------------

Runtime / link:

* `HTSlib <https://github.com/samtools/htslib>`_ (BAM I/O)
* `OR-Tools <https://github.com/google/or-tools>`_ (some solvers)
* `CLI11 <https://github.com/CLIUtils/CLI11>`_ (fetched by CMake)

Install script:

.. code-block:: sh

   ./scripts/install_libs.sh install

IDE setup (VS Code)
-------------------

Recommended extensions:

* clangd (LLVM)
* clang-format (Xaver Hellauer)
* CMake (twxs)
* CMake Tools (Microsoft)
* cmake-format (cheshirekow)

Example settings:

.. code-block:: json

   {
     "editor.formatOnSave": true,
     "[cpp]": {
       "editor.defaultFormatter": "xaver.clang-format"
     },
     "cmake.options.statusBarVisibility": "visible",
     "C_Cpp.default.configurationProvider": "clangd",
     "C_Cpp.intelliSenseEngine": "disabled",
     "C_Cpp.codeAnalysis.clangTidy.enabled": true,
     "C_Cpp.codeAnalysis.clangTidy.path": "/usr/bin/clang-tidy",
     "C_Cpp.codeAnalysis.clangTidy.useBuildPath": true,
     "C_Cpp.codeAnalysis.clangTidy.args": ["-p", "./build/"],
     "clang-format.executable": "/usr/bin/clang-format",
     "clangd.path": "/usr/bin/clangd",
     "cmakeFormat.exePath": "/usr/bin/cmake-format",
     "[cmake]": {
       "editor.defaultFormatter": "cheshirekow.cmake-format"
     }
   }

On first open, pick the newest GCC CMake kit. Use **CMake: Build** / **CMake: Run** instead of the default C++ run button so libraries link correctly.

Known issues
------------

* ``.cu`` files may not see packages added via FetchContent in clangd (compile works; IDE may show false errors).
