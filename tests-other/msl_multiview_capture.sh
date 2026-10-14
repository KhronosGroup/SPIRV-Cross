#!/usr/bin/env bash
# Copyright 2026 Jean-Philippe Meunier
# SPDX-License-Identifier: Apache-2.0
# CPU compilation and Metal 2.4 syntax only; no GPU device or submission.
set -euo pipefail
root=$(cd "$(dirname "$0")/.." && pwd)
build=$(cd "${1:?BUILD required}" && pwd)
output=${2:?OUTPUT required}
mode=${3:-after}
mkdir -p "$output"
for fixture in vert effects view-only; do
    defines=()
    if [[ $fixture == effects ]]; then defines+=(-DEFFECTS=1); fi
    if [[ $fixture == view-only ]]; then defines+=(-DVIEW_ONLY=1); fi
    glslangValidator -V --target-env vulkan1.1 ${defines[@]+"${defines[@]}"} "$root/tests-other/msl_multiview_capture.vert" -o "$output/$fixture.spv"
    spirv-val --target-env vulkan1.1 "$output/$fixture.spv"
done
glslangValidator -V --target-env vulkan1.1 "$root/tests-other/msl_multiview_capture.frag" -o "$output/frag.spv"
spirv-val --target-env vulkan1.1 "$output/frag.spv"
cp "$root/tests-other/msl_capture_layout_complex.spv" "$output/application-layer.spv"
spirv-val --target-env vulkan1.1 "$output/application-layer.spv"
defines=()
if [[ $mode == before ]]; then defines+=(-DMULTIVIEW_BEFORE); fi
xcrun clang++ -std=c++14 ${defines[@]+"${defines[@]}"} -I "$root" "$root/tests-other/msl_multiview_capture.cpp" "$build/libspirv-cross-msl.a" "$build/libspirv-cross-glsl.a" "$build/libspirv-cross-core.a" -o "$output/test"
"$output/test" "$output" > "$output/api.log" 2>&1 || { cat "$output/api.log"; exit 1; }
if [[ $mode == before ]]; then
    failures=0
    for name in vert-1-1-0 vert-1-2-0 vert-1-3-0 view-only-1-1-0 vert-1-0-1 vert-0-1-1; do
        if xcrun metal -x metal -std=macos-metal2.4 -fno-modules -fsyntax-only "$output/capture-$name.metal" > "$output/$name.log" 2>&1; then
            echo "Expected baseline compilation failure: $name" >&2; exit 1
        fi
        case "$name" in
            vert-1-0-1) grep -q 'expression is not assignable' "$output/$name.log" ;;
            vert-0-1-1) grep -q "invalid 'base_instance' attribute" "$output/$name.log" ;;
            *) grep -q "use of undeclared identifier 'gl_InstanceIndex'" "$output/$name.log" ;;
        esac
        failures=$((failures+1))
    done
    echo "REPRODUCED: $failures real layered capture Metal syntax failures; default replay rejects multiview."
    exit 0
fi
xcrun clang++ -std=c++14 -fsanitize=address,undefined -fno-sanitize-recover=all "$output/mapping.cpp" -o "$output/mapping"
"$output/mapping"
# Prove the application-output assertions detect missing/double normalization and lost base reads.
# Mutate only copied application stores, never builtin remapping or physical record addressing.
for mutation in instance-missing instance-double vertex-missing vertex-double base-instance base-vertex; do
    case "$mutation" in
        instance-missing) edit='/out.appInstance = /s/ - gl_BaseInstance//'; diagnostic=InstanceIndex ;;
        instance-double) edit='/out.appInstance = /s/ - gl_BaseInstance/ - gl_BaseInstance - gl_BaseInstance/'; diagnostic=InstanceIndex ;;
        vertex-missing) edit='/out.appVertex = /s/ - gl_BaseVertex//'; diagnostic=VertexIndex ;;
        vertex-double) edit='/out.appVertex = /s/ - gl_BaseVertex/ - gl_BaseVertex - gl_BaseVertex/'; diagnostic=VertexIndex ;;
        base-instance) edit='s/out.appBaseInstance = int(gl_BaseInstance);/out.appBaseInstance = 0;/'; diagnostic=BaseInstance ;;
        base-vertex) edit='s/out.appBaseVertex = int(gl_BaseVertex);/out.appBaseVertex = 0;/'; diagnostic=BaseVertex ;;
    esac
    sed "$edit" "$output/mapping.cpp" > "$output/mutation-$mutation.cpp"
    if cmp -s "$output/mapping.cpp" "$output/mutation-$mutation.cpp"; then
        echo "Mutation did not change an emitted assignment: $mutation" >&2; exit 1
    fi
    xcrun clang++ -std=c++14 -fsanitize=address,undefined -fno-sanitize-recover=all "$output/mutation-$mutation.cpp" -o "$output/mutation-$mutation"
    if "$output/mutation-$mutation" > "$output/mutation-$mutation.log" 2>&1; then
        echo "Application assertion missed mutation: $mutation" >&2; exit 1
    fi
    grep -qx "emitted application $diagnostic" "$output/mutation-$mutation.log"
done
count=0
: > "$output/syntax.log"
for shader in "$output"/*.metal; do
    if ! xcrun metal -x metal -std=macos-metal2.4 -fno-modules -fsyntax-only "$shader" >> "$output/syntax.log" 2>&1; then
        echo "$shader" >&2; cat "$output/syntax.log" >&2; exit 1
    fi
    count=$((count+1))
done
echo "PASS: API checks, emitted application stores and physical mapping under ASan/UBSan, six rejected application mutations, $count Metal 2.4 syntax checks."
