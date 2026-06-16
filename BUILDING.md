# Building

Requirements:

- cmake
- SDL2
- rtmidi (Linux, Mac only)

Tested compilers:

- msvc 19.39.33523
- clang 19.1.7
- gcc 14.2.0

Full build

```bash
git clone git@github.com:jcmoyer/Nuked-SC55.git
cd Nuked-SC55
mkdir build
cd build
cmake -DCMAKE_BUILD_TYPE=Release ..
cmake --build .
```

If you're building a binary to only run on your local machine, consider adding
`-DCMAKE_CXX_FLAGS="-march=native -mtune=native"
-DCMAKE_INTERPROCEDURAL_OPTIMIZATION=ON` to the first cmake command above to
enable more optimizations.

After building, you can create a self-contained install with any required files
in their correct locations under `<path>`:

```bash
cmake --install . --prefix=<path>
```

### Windows

For builds using msvc you will most likely need to pass
`-DCMAKE_PREFIX_PATH=<path>` where `<path>` points to a directory containing
SDL2, and optionally rtmidi (only when `-DUSE_RTMIDI=ON`).

cmake is expecting to find `<path>/SDL2-X.YY.Z/cmake/sdl2-config.cmake`.

For builds in an msys2 environment, installing SDL2 via pacman should be
enough.

#### ASIO (optional)

To enable ASIO support, pass `-DNUKED_ENABLE_ASIO=ON` and
`-DNUKED_ASIO_SDK_DIR=<path>` where `<path>` points to the extracted ASIO SDK
obtained from [here](https://www.steinberg.net/developers/).

# Development

Requirements:

- Python 3
- [Catch2 v3.7.0](https://github.com/catchorg/Catch2) installed in
  `CMAKE_PREFIX_PATH`

There is a test suite that makes sure new commits don't change existing
behavior. It is expected that all tests pass for every commit on master.

You can run the test suite by configuring with the following cmake variables:

- `-DNUKED_ENABLE_TESTS=ON`: when set, the following variables must all be set
  as well.
- `-DNUKED_TEST_ROMDIR=<path>`: `<path>` should point to a directory containing
  the romsets listed below.
- `-DNUKED_TEST_JV880_NVRAM=<path>`: `<path>` should point to a file containing
  nvram dumped from the JV-880 immediately after it has been reset to the
  factory preset. This can be obtained by launching nuked-sc55 with `--romset
  jv880 --nvram <path>`. Once the emulator has started, press `T` to enter the
  utility menu, then press `.` until `Util:Factory preset` appears. Press `G`
  twice and close the emulator. The file `<path>` should contain a 32K nvram
  dump. Note that the actual filename will have a number appended to it. This
  is the emulator instance number and should **not** be included in the
  filename passed to cmake.

Most romsets have a test suite; see `test/integration/CMakeLists.txt` for the
complete list.

The file containing the JV-880 factory preset nvram dump should have the
following SHA-256 hash:

```
d5da784546f9fd482c82beb366c527f313e8ea81bc9039dbb8c531197aa6d207 *jv880/nvram0
```

After cmake has configured the build, you can run the test suite:

```
$ cmake --build . --config Release && ctest . -C Release
```

Note that these tests take a long time to finish individually, so you may want
to pass `-j` to run them in parallel.
