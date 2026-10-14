#!/usr/bin/env bash
# Copyright 2026 Jean-Philippe Meunier
# SPDX-License-Identifier: Apache-2.0
#
# Licensed under the Apache License, Version 2.0 (the License);
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#     http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an AS IS BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.

# CPU code generation and Metal syntax only. Never creates a GPU device or pipeline.
set -euo pipefail
if [[ $# -lt 1 || $# -gt 2 ]]; then
    echo "Usage: $0 BUILD [OUTPUT]" >&2
    exit 1
fi
build=$(cd "$1" && pwd)
output=${2:-"$build/barycentric-metal"}
mkdir -p "$output"
if ! "$build/spirv-cross-msl-barycentric-test" "$build/msl_barycentric_portable.spv" "$build/msl_barycentric_interpolate.spv" "$build/msl_barycentric_block.spv" "$build/msl_barycentric_copied_pointer.spv" "$build/msl_barycentric_explicit_only.spv" "$build/msl_barycentric_mixed_block.spv" "$build/msl_barycentric_mixed_frontfacing.spv" "$build/msl_barycentric_mixed_builtins.spv" "$build/msl_barycentric_multiword_mask.spv" "$build/msl_barycentric_inactive_block.spv" "$build/msl_barycentric_empty_root.spv" "$build/msl_barycentric_per_vertex.spv" "$output" > "$output/api.log" 2>&1; then
    cat "$output/api.log" >&2
    exit 1
fi
count=0
: > "$output/syntax.log"
for shader in "$output"/*.metal; do
    if ! xcrun metal -x metal -std=macos-metal2.4 -fsyntax-only -fno-modules "$shader" >> "$output/syntax.log" 2>&1; then
        cat "$output/syntax.log" >&2
        exit 1
    fi
    count=$((count + 1))
done
printf 'API checks and %s Metal 2.4 syntax checks passed (CPU only).\n' "$count"
