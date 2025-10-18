ink / A Native Whiteboard App
============

![demo](./screenshot/1.png)


> [!WARNING]
> This software is not finished, expect missing features and bugs
> Built with C and Vulkan, so a decent performance should be expected

## Development Setup Instructions

**Note**: Currently, only x64 linux and Windows development are supported (check the Releases page for downloads).

### Clone Repo

```
git clone --recurse-submodules https://github.com/pickle-prick/ink
```

### Linux Setup

#### 1. Installing the Required Tools

* linux
* gcc or clang
* glslc
* xxd
* vulkan-headers
* vulkan-validation-layers

#### 2. Building

Within this terminal, `cd` to the root directory of the codebase, and just run the `build.sh` script:

```
./build.sh ink clang release
```

### Windows Setup

#### 1. Installing the Required Tools

In order to work with the codebase, you'll need the [Microsoft C/C++ Build Tools
v15 (2017) or later](https://aka.ms/vs/17/release/vs_BuildTools.exe), for both
the Windows SDK and the MSVC/Clang compiler and linker.

Install [Vulkan SDK](https://vulkan.lunarg.com/sdk/home) for windows

#### 2. Build Environment Setup

Building the codebase can be done in a terminal which is equipped with the
ability to call either MSVC or Clang from command line.

This is generally done by calling `vcvarsall.bat x64`, which is included in the
Microsoft C/C++ Build Tools. This script is automatically called by the `x64
Native Tools Command Prompt for VS <year>` variant of the vanilla `cmd.exe`. If
you've installed the build tools, this command prompt may be easily located by
searching for `Native` from the Windows Start Menu search.

### 3. Building

Within this terminal, `cd` to the root directory of the codebase, and just run
the `build.bat` script:

```
build ink msvc release
```

## Bug Report

If everything worked correctly, there will be a `build` folder in the root
level of the codebase, and it will contain a freshly-built `ink` executable.

Join discrod https://discord.gg/6tzEKgKG for bug report or feature suggestions (or you can just create a issue)

Any issues/suggestions are appreciated

Some known issues:
* bad tablet support (I need to buy a tablet)
* crashes on window resizing (in some cases)
* can't copy/paste element
* no redo/undo
* crashes on startup (in some cases)
