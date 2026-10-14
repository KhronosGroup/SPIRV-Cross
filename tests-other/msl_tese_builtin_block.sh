#!/usr/bin/env bash
# Copyright 2026 Jean-Philippe Meunier
# SPDX-License-Identifier: Apache-2.0
set -euo pipefail

compiler=$1
assembler=$2
validator=$3
fixtures=$4
# Keep generated fixtures and diagnostics when an evidence directory is requested.
if [[ -n ${SPIRV_CROSS_TESE_EVIDENCE_DIR:-} ]]; then
    mkdir -p "$SPIRV_CROSS_TESE_EVIDENCE_DIR"
    tmp=$(cd "$SPIRV_CROSS_TESE_EVIDENCE_DIR" && pwd)
else
    tmp=$(mktemp -d)
    trap 'rm -rf "$tmp"' EXIT
fi

for name in msl_tese_input_builtin_block msl_tese_builtin_block; do
    "$assembler" --target-env vulkan1.1 "$fixtures/$name.spvasm" -o "$tmp/$name.spv"
    "$validator" --target-env vulkan1.1 "$tmp/$name.spv"
    normal=$("$compiler" "$tmp/$name.spv" --msl --msl-version 20400)
    compute=$("$compiler" "$tmp/$name.spv" --msl --msl-version 20400 --msl-capture-output --msl-raw-buffer-tese-input --msl-tese-as-compute 3)
    [[ "$normal" == *'float3 gl_TessCoord [[position_in_patch]]'* ]]
    [[ "$normal" == *'out.gl_Position = float4(gl_TessCoord, 1.0);'* ]]
    [[ "$normal" != *'struct main0_in'* ]]
    [[ "$compute" == *'float3 gl_TessCoord = as_type<float3>(uint3(spvTessRecord[0], spvTessRecord[1], spvTessRecord[2]));'* ]]
    [[ "$compute" == *'out.gl_Position = float4(gl_TessCoord, 1.0);'* ]]
    [[ "$compute" != *'struct main0_in'* ]]
done

name=msl_tese_primitive_builtin_block
"$assembler" --target-env vulkan1.1 "$fixtures/$name.spvasm" -o "$tmp/$name.spv"
"$validator" --target-env vulkan1.1 "$tmp/$name.spv"
normal=$("$compiler" "$tmp/$name.spv" --msl --msl-version 20400)
compute=$("$compiler" "$tmp/$name.spv" --msl --msl-version 20400 --msl-capture-output --msl-raw-buffer-tese-input --msl-tese-as-compute 3)
[[ "$normal" == *'uint gl_PrimitiveID [[patch_id]]'* ]]
[[ "$normal" == *'out.gl_Position = float4(gl_TessCoord.x + float(gl_PrimitiveID), gl_TessCoord.yz, 1.0);'* ]]
[[ "$compute" == *'uint gl_PrimitiveID = spvTessRecord[3];'* ]]
[[ "$compute" == *'out.gl_Position = float4(gl_TessCoord.x + float(gl_PrimitiveID), gl_TessCoord.yz, 1.0);'* ]]
[[ "$normal" != *'struct main0_in'* && "$compute" != *'struct main0_in'* ]]

sed 's/OpExecutionMode %main Triangles/OpExecutionMode %main Quads/' "$fixtures/msl_tese_input_builtin_block.spvasm" > "$tmp/quad.spvasm"
"$assembler" --target-env vulkan1.1 "$tmp/quad.spvasm" -o "$tmp/quad.spv"
"$validator" --target-env vulkan1.1 "$tmp/quad.spv"
quad=$("$compiler" "$tmp/quad.spv" --msl --msl-version 20400)
[[ "$quad" == *'float2 gl_TessCoordIn [[position_in_patch]]'* ]]
[[ "$quad" == *'float3 gl_TessCoord = float3(gl_TessCoordIn.x, gl_TessCoordIn.y, 0.0);'* ]]

metal_available=false
if command -v xcrun >/dev/null && xcrun --find metal >/dev/null 2>&1; then
    metal_available=true
fi
for fixture in helper patch patch_helper; do
    for domain in triangle quad quad_lower_left; do
        name="${fixture}_${domain}"
        if [[ $domain == triangle ]]; then
            cp "$fixtures/msl_tese_builtin_block_$fixture.spvasm" "$tmp/$name.spvasm"
        else
            sed 's/OpExecutionMode %main Triangles/OpExecutionMode %main Quads/' "$fixtures/msl_tese_builtin_block_$fixture.spvasm" > "$tmp/$name.spvasm"
        fi
        "$assembler" --target-env vulkan1.1 "$tmp/$name.spvasm" -o "$tmp/$name.spv"
        "$validator" --target-env vulkan1.1 "$tmp/$name.spv"
        for mode in normal compute3 compute7; do
            # TES compute currently supports triangle domains only.
            if [[ $domain != triangle && $mode != normal ]]; then
                continue
            fi
            args=(--msl --msl-version 20400)
            if [[ $mode != normal ]]; then
                args+=(--msl-capture-output --msl-raw-buffer-tese-input --msl-tese-as-compute "${mode#compute}")
            elif [[ $domain == quad_lower_left ]]; then
                args+=(--msl-domain-lower-left)
            fi
            output="$tmp/$name-$mode.metal"
            "$compiler" "$tmp/$name.spv" "${args[@]}" > "$output"
            msl=$(cat "$output")
            [[ "$msl" != *'spvIn'* && "$msl" != *'spvPatchIn'* ]]
            if [[ $fixture != patch ]]; then
                [[ "$msl" == *'float4 read_builtins(thread float3& gl_TessCoord, thread uint& gl_PrimitiveID'* ]]
                [[ "$msl" == *'return read_builtins(gl_TessCoord, gl_PrimitiveID'* ]]
                [[ "$msl" == *'out.gl_Position = forward_builtins(gl_TessCoord, gl_PrimitiveID'* ]]
            fi
            if [[ $fixture == patch* ]]; then
                if [[ $mode == normal ]]; then
                    [[ "$msl" == *'uint gl_PatchVerticesIn = patchIn.gl_in.size();'* ]]
                    [[ "$msl" != *'[[buffer('* ]]
                else
                    [[ "$msl" == *"uint gl_PatchVerticesIn = ${mode#compute};"* ]]
                    [[ "$msl" != *'struct main0_in'* ]]
                fi
            fi
            if [[ $mode != normal ]]; then
                [[ "$msl" == *'spvTessInvocations + 4ul + ulong(spvTessInvocation.x) * 8ul'* ]]
                [[ "$msl" == *'spvOut[ulong(spvTessRecord[4])]'* ]]
                [[ "$msl" == *'spvTessInvocations [[buffer(29)]]'* && "$msl" == *'spvOut [[buffer(28)]]'* ]]
            fi
            if $metal_available; then
                xcrun metal -x metal -std=macos-metal2.4 -fno-modules -fsyntax-only "$output" > "$output.log" 2>&1 || { cat "$output.log" >&2; exit 1; }
            fi
        done
    done
done
# No control-point input: a zero stage-in ID must not make writable SSBOs const.
name=msl_tese_builtin_block_ssbo
"$assembler" --target-env vulkan1.1 "$fixtures/$name.spvasm" -o "$tmp/$name.spv"
"$validator" --target-env vulkan1.1 "$tmp/$name.spv"
for patch_size in 3 7; do
    output="$tmp/$name-compute$patch_size.metal"
    "$compiler" "$tmp/$name.spv" --msl --msl-version 20400 --msl-capture-output --msl-raw-buffer-tese-input --msl-tese-as-compute "$patch_size" > "$output"
    if $metal_available; then
        xcrun metal -x metal -std=macos-metal2.4 -fno-modules -fsyntax-only "$output" > "$output.log" 2>&1 || { cat "$output.log" >&2; exit 1; }
    fi
    msl=$(cat "$output")
    [[ "$msl" == *'kernel void main0(device Storage& ssbo [[buffer(0)]]'* ]]
    [[ "$msl" == *'ssbo.value = 1.0;'* ]]
    [[ "$msl" != *'spvIn'* && "$msl" != *'spvPatchIn'* ]]
done
if ! $metal_available; then
    echo 'SKIP: Metal syntax validation (xcrun metal unavailable)'
fi
