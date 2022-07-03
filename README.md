# Core Patch Library

This library contains core functionality that's used by the Serious Sam Classic Patch.

It's utilized primarily in [`GameExecutable`](https://github.com/SamClassicPatch/GameExecutable) and [`DedicatedServer`](https://github.com/SamClassicPatch/DedicatedServer) projects.

## Building

Before building the code, make sure to load in the submodules. Use `git submodule update --init --recursive command` to load files for all submodules.

To compile the executable, you'll need to use a compiler from Microsoft Visual C++ 6.0.

Full guide: https://github.com/DreamyCecil/SeriousSam_SDK107#building

## License

This project is licensed under the GNU GPL v2 (see LICENSE file).

Some of the code included with the SDK may not be licensed under the GNU GPL v2:

* DirectX8 SDK (Headers & Libraries) (`d3d8.h`, `d3d8caps.h` and `d3d8types.h` located in `Includes` folder) by Microsoft
