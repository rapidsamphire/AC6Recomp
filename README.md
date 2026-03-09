# AC6Recomp

> [!CAUTION]
> This project is a work in progress, so a stable experience isn't guaranteed yet. It progresses in-game, but more testing and modifications are needed.

Recompiled using the [ReXGlue SDK](https://github.com/rexglue/rexglue-sdk).

## Prerequisites

- [CMake](https://cmake.org/) 3.25+
- [Ninja](https://ninja-build.org/)
- [Clang](https://releases.llvm.org/) (LLVM/Clang toolchain)
- A legally obtained copy of the game.

## Building

Clone the repository with submodules:

```bash
git clone --recursive https://github.com/rapidsamphire/AC6Recomp.git
cd AC6Recomp
```

If you already cloned without `--recursive`:

```bash
git submodule update --init --recursive
```

### Assets

You need the game's original Xbox 360 disc image (ISO). Tools to extract it can be found [here](https://consolemods.org/wiki/Xbox:ISO_Extraction_%26_Repacking).

After extraction, drop both the game assets and `default.xex` into the `assets/` directory.

### Generate recompiled code

```bash
cmake --preset win-amd64-relwithdebinfo
cmake --build --preset win-amd64-relwithdebinfo --target ac6recomp_codegen
```

### Build

```bash
cmake --build --preset win-amd64-relwithdebinfo
```

(Relwithdebinfo is the recommended build preset as of now.)

The executable will be in `out/build/win-amd64-relwithdebinfo/`.

### Running

```bash
./out/build/win-amd64-relwithdebinfo/ac6recomp assets
```

### Linux

Replace `win-amd64-relwithdebinfo` with `linux-amd64-relwithdebinfo` in the commands above.
