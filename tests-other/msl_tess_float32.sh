#!/usr/bin/env bash
# Copyright 2026 Jean-Philippe Meunier
# SPDX-License-Identifier: Apache-2.0
set -euo pipefail
root=$(cd "$(dirname "$0")/.." && pwd)
build=$(cd "${1:?BUILD required}" && pwd)
mkdir -p "${2:?OUTPUT required}"
output=$(cd "$2" && pwd)
before=${3:?PRE_CHANGE_COMPILER required}
mkdir -p "$output/tmp"
export TMPDIR="$output/tmp"
"$build/spirv-cross-msl-tess-float32-test" "$build/msl_tess_float32.tesc.spv" "$build/msl_tess_float32.tese.spv" "$build/msl_tess_float32_copy.spv" "$build/msl_tess_float32_tes_copy.spv" "$build/msl_tess_float32_block.spv" "$build/msl_tess_float32_parameter.spv" "$root/tests-other/msl_capture_layout_simple.spv" "$output" | tee "$output/api.log"
for fixture in tcs tes copy tes-copy; do
    for mode in 0 1 2 3; do
        xcrun metal -x metal -std=macos-metal2.4 -fno-modules -fsyntax-only "$output/$fixture$mode.metal" > "$output/$fixture$mode.syntax.log" 2>&1
    done
done
for fixture in reversed-disabled reversed-distinct reversed-tcs tcs-argument-buffers tes-argument-buffers; do
    xcrun metal -x metal -std=macos-metal2.4 -fno-modules -fsyntax-only "$output/$fixture.metal" > "$output/$fixture.syntax.log" 2>&1
done
"$build/spirv-cross" "$build/msl_tess_float32.tesc.spv" --msl --msl-version 20400 --msl-tessellation-factors-float32 --output "$output/cli.metal"
xcrun metal -x metal -std=macos-metal2.4 -fno-modules -fsyntax-only "$output/cli.metal" > "$output/cli.syntax.log" 2>&1
"$before" "$build/msl_tess_float32_copy.spv" --msl --msl-version 20400 --output "$output/before-half.metal"
"$build/spirv-cross" "$build/msl_tess_float32_copy.spv" --msl --msl-version 20400 --output "$output/after-half.metal"
cmp "$output/before-half.metal" "$output/after-half.metal"
xcrun clang++ -std=c++14 -fobjc-arc -framework Foundation -framework Metal "$root/tests-other/msl_tess_float32_runtime.mm" -o "$output/runtime" > "$output/runtime-build.log" 2>&1
MTL_DEBUG_LAYER=1 MTL_SHADER_VALIDATION=1 "$output/runtime" "$output" 2>&1 | tee "$output/runtime.log"
echo 'PASS: 22 Metal syntax checks, CLI, default half comparison and GPU precision/ABI control.'
