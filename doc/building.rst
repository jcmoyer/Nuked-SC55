########
Building
########

Requirements:

- cmake
- SDL2
- rtmidi (on Linux or Mac)

Tested compilers:

- msvc 19.39.33523
- clang 19.1.7
- gcc 14.2.0

Full build:

.. code:: bash

  git clone git@github.com:jcmoyer/Nuked-SC55.git
  cd Nuked-SC55
  mkdir build
  cd build
  cmake -DCMAKE_BUILD_TYPE=Release ..
  cmake --build .

If you're building a binary to run only on your local machine, consider adding
``-DCMAKE_CXX_FLAGS="-march=native -mtune=native"
-DCMAKE_INTERPROCEDURAL_OPTIMIZATION=ON`` to the first cmake command to enable
more optimizations.

After building, you can create a self-contained install with any required files
in their correct locations under ``<path>``::

  cmake --install . --prefix=<path>

Windows-specific instructions
=============================

MSYS2 / MINGW64
---------------

Compiling on Windows is easiest under an `MSYS2`_ environment. Binary
distributions from the releases page are built using the MINGW64
environment. In this case it is sufficient to install cmake and SDL2 via pacman
before building::

  pacman -S mingw-w64-x86_64-cmake mingw-w64-x86_64-SDL2

.. _MSYS2: https://www.msys2.org/

MSVC
----

Builds using MSVC should be done under the developer command prompt that comes
installed with Visual Studio. The name resembles something like ``x64 Native
Tools Command Prompt for VS 2022`` and can be accessed from the start menu in a
typical installation. This environment should have cmake pre-installed. If not,
you might need to run the Visual Studio Installer and add it to your
installation.

You will need to pass ``-DCMAKE_PREFIX_PATH=<path>`` where ``<path>`` points to
a directory containing an SDL2 installation. You can either download a release
`here <SDL2-releases_>`_ or build SDL from source. If downloading a release,
make sure it is the correct version and that the filename contains ``devel``
and ``VC``.

cmake is expecting to find ``<path>/SDL2-X.Y.Z/cmake/sdl2-config.cmake``.

.. _SDL2-releases: https://github.com/libsdl-org/SDL/releases

ASIO support (optional)
-----------------------

To enable ASIO support, pass ``-DNUKED_ENABLE_ASIO=ON`` and
``-DNUKED_ASIO_SDK_DIR=<path>`` where ``<path>`` points to the extracted ASIO
SDK obtained `here <asio-sdk_>`_.

.. _asio-sdk: https://www.steinberg.net/developers/

Development instructions
========================

Requirements:

- Python 3
- `Catch2 v3.7.0 <catch2_>`_ installed in ``CMAKE_PREFIX_PATH``

.. _catch2: https://github.com/catchorg/Catch2

This fork has two test suites. The first is a set of integration tests that
hash emulator output to ensure that new commits don't change how audio is
rendered. The second is a set of unit tests for testing smaller mockable
features. It is expected that all tests pass for each commit on master.

You can run the test suite by configuring with the following cmake variables:

- ``-DNUKED_ENABLE_TESTS=ON``: **required** to enable testing. When set, the
  following variables **must** all be set as well.
- ``-DNUKED_TEST_ROMDIR=<path>``: ``<path>`` should point to a directory
  containing romsets. The directory structure does not matter; romsets are
  discovered automatically and recursively.
- ``-DNUKED_TEST_JV880_NVRAM=<path>``: ``<path>`` should point to a file
  containing nvram dumped from the JV-880 immediately after it has been reset
  to the factory preset. This can be obtained by launching nuked-sc55 with
  ``--romset jv880 --nvram <path>``. Once the emulator has started, press ``T``
  to enter the utility menu, then press ``.`` until ``Util:Factory preset``
  appears. Press ``G`` twice and close the emulator. The file ``<path>`` should
  contain a 32K nvram dump. Note that the actual filename will have a number
  appended to it. This is the emulator instance number.

The JV-880 nvram dump should have the following SHA-256 hash::

  d5da784546f9fd482c82beb366c527f313e8ea81bc9039dbb8c531197aa6d207

After cmake has configured the build, you can run the test suite::

  $ cmake --build . --config Release && ctest . -C Release

Note that these tests take a long time to finish individually, so you may want
to pass ``-j`` to run them in parallel.
