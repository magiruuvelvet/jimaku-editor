# Linux & \*BSD Build Instructions

## Build Dependencies

 - CMake 3.14 or higher (may work with a lower version)
 - a recent C++17 compiler (clang 8+, gcc 9+)

## Preparation

Install external dependencies and header files for development.

On most distributions packages are split into multiple components.

Required: `qtcore`, `qtgui`, `imagemagick` (with C++ bindings), `libpng`

**Note to distributors:** Embedded dependencies are **modified** to fit the
needs of the project. Using shared versions them is unsupported and may
cause problems or breakage. Ignoring this notice causes all support requests
to be silently ignored and closed. You have been warned.

## CMake Build Options

 - `-DINCLUDE_GIT_TRACKING` (default: `ON`):
   If the full git version with commit hash and dirty status should
   be included in the application version.
   For release builds with a clean release tarball, this is recommended
   to turn off. This option also must be turned off when git is not
   available on the build machine, otherwise CMake throws an error.

 - `-DENABLE_TESTS` (default: `ON`):
   Whenever unit tests should be build. Unit tests contain hardcoded
   paths generated during the CMake process. Moving the source code
   repository makes unit tests fail. The unit test binary itself can
   be freely moved.

## Building

An out of source tree build is recommended.

```sh
mkdir build && cd build
cmake (build options) ..
make
```

There is currently no `install` target defined. To install the project,
manually copy all binaries inside the `build/bin` directory (excluding
unit tests) to your preferred `$PATH`.
