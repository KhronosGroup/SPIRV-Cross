# SPIR2CROSS

SPIR2CROSS is a tool designed for parsing and converting SPIR-V to other shader languages.

## Features

  - Convert SPIR-V to readable, usable and efficient GLSL
  - Convert SPIR-V to debuggable C++ [EXPERIMENTAL]
  - Reflection API to simplify the creation of Vulkan pipeline layouts
  - Reflection API to modify and tweak OpDecorations
  - Supports "all" of vertex, fragment, tessellation, geometry and compute shaders.

SPIR2CROSS tries hard to emit readable and clean output from the SPIR-V.
The goal is to emit GLSL that looks like it was written by a human and not awkward IR/assembly-like code.

NOTE: Individual features are expected to be mostly complete, but it is possible that certain obscure GLSL features are not yet supported.
However, most missing features are expected to be "trivial" improvements at this stage.

Occasionally, missing features is due to glslangValidator's lack of proper support for that feature making testing hard.

## Building

SPIR2CROSS has been tested on Linux, OSX and Windows.

### Linux and OSX

Just run `make` on the command line. A recent GCC (4.8+) or Clang (3.x+) compiler is required as SPIR2CROSS uses C++11 extensively.

### Windows

MinGW-w64 based compilation works with `make`, and an MSVC 2013 solution is also included.

## Usage

### Creating a SPIR-V file from GLSL with glslang

```
glslangValidator -H -V -o test.spv test.frag
```

### Converting a SPIR-V file to GLSL ES

```
glslangValidator -H -V -o test.spv shaders/comp/basic.comp
./spir2cross --version 310 --es test.spv
```

#### Converting to desktop GLSL

```
glslangValidator -H -V -o test.spv shaders/comp/basic.comp
./spir2cross --version 330 test.spv --output test.comp
```

#### Disable prettifying optimizations

```
glslangValidator -H -V -o test.spv shaders/comp/basic.comp
./spir2cross --version 310 --es test.spv --output test.comp --force-temporary
```

## ABI concerns

### SPIR-V headers

The current repository uses the latest SPIR-V and GLSL.std.450 headers.
SPIR-V files created from older headers could have ABI issues.

## Regression testing

In shaders/ a collection of shaders are maintained for purposes of regression testing.
The current reference output is contained in reference/.
`./test_shaders.py shaders` can be run to perform regression testing.

### Updating regression tests

When legitimate changes are found, use `--update` flag to update regression files.
Otherwise, `./test_shaders.py` will fail with error code.

### Mali Offline Compiler cycle counts

To obtain a CSV of static shader cycle counts before and after going through spir2cross, add
`--malisc` flag to `./test_shaders`. This requires the Mali Offline Compiler to be installed in PATH.

