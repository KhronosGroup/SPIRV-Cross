#!/usr/bin/env bash
# Copyright 2026 Jean-Philippe Meunier
# SPDX-License-Identifier: Apache-2.0
# Existing configured CMake build required; all generated evidence remains under OUTPUT.
set -euo pipefail
root=$(cd "$(dirname "$0")/.." && pwd)
build=$(cd "${1:?BUILD required}" && pwd)
mkdir -p "${2:?OUTPUT required}"
output=$(cd "$2" && pwd)
mkdir -p "$output/tmp"
export TMPDIR="$output/tmp"
"$build/spirv-cross-msl-tese-compute-test" "$build" "$root/tests-other/msl_capture_layout_simple.spv" "$build/msl_tese_pervertex.spv" "$output" | tee "$output/api.log"
cmp "$output/triangle.layout" "$output/block.layout"
for fixture in triangle constant resource block; do
    xcrun metal -x metal -std=macos-metal2.4 -fno-modules -fsyntax-only "$output/$fixture.metal" > "$output/$fixture-syntax.log" 2>&1
done
xcrun metal -x metal -std=macos-metal2.4 -fno-modules -fsyntax-only "$root/tests-other/msl_tese_topology.metal" > "$output/topology-syntax.log" 2>&1
xcrun metal -x metal -std=macos-metal2.4 -fno-modules -fsyntax-only "$output/tese-replay.metal" > "$output/tese-replay-syntax.log" 2>&1
xcrun metal -x metal -std=macos-metal2.4 -fno-modules -fsyntax-only "$output/tese-pervertex.metal" > "$output/tese-pervertex-syntax.log" 2>&1
xcrun metal -x metal -std=macos-metal2.4 -fno-modules -fsyntax-only "$output/tese-pervertex-primitive.metal" > "$output/tese-pervertex-primitive-syntax.log" 2>&1
"$build/spirv-cross" "$build/msl_tese_compute_triangle.spv" --msl --msl-version 20400 --msl-raw-buffer-tese-input --msl-capture-output --msl-tese-as-compute 3 --output "$output/cli.metal"
xcrun metal -x metal -std=macos-metal2.4 -fno-modules -fsyntax-only "$output/cli.metal" > "$output/cli-syntax.log" 2>&1
if "$build/spirv-cross" "$build/msl_tese_compute_triangle.spv" --msl --msl-version 20400 --msl-capture-output > "$output/legacy-after.log" 2>&1; then
    echo 'Legacy TES capture should fail' >&2; exit 1
fi
grep -q 'TES capture requires tese_as_compute' "$output/legacy-after.log"
xcrun clang++ -std=c++14 -fobjc-arc -framework Foundation -framework Metal "$root/tests-other/msl_tese_compute_runtime.mm" -o "$output/runtime"
for fixture in triangle block; do
    "$output/runtime" "$output" "$fixture" "$root/tests-other/msl_tese_topology.metal" 2>&1 | tee "$output/runtime-$fixture.log"
    "$output/runtime" "$output" "$fixture" "$root/tests-other/msl_tese_topology.metal" 3 2>&1 | tee "$output/runtime-$fixture-factor3.log"
done
echo 'PASS: nine Metal 2.4 syntax checks including TES replay/PerVertex fragment, CLI rejection and GPU-produced TES rendering at factors 1/2 and 1/3.'
