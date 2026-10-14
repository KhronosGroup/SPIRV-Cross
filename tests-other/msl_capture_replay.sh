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
source=$(cd "$(dirname "$0")" && pwd)
output=${2:-"$build/replay-metal"}
mkdir -p "$output"
glslangValidator -V --target-env vulkan1.1 "$source/msl_capture_replay_dense.vert" -o "$output/msl_capture_replay_dense.spv" > "$output/dense-build.log" 2>&1
spirv-val --target-env vulkan1.1 "$output/msl_capture_replay_dense.spv" >> "$output/dense-build.log" 2>&1
if ! "$build/spirv-cross-msl-capture-replay-test" "$source/msl_capture_layout_simple.spv" "$source/msl_capture_layout_complex.spv" "$source/msl_capture_replay_effects.spv" "$source/msl_capture_replay_unsupported.spv" "$source/msl_capture_layout_unsupported.spv" "$output" "$output/msl_capture_replay_dense.spv" > "$output/api.log" 2>&1; then
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
