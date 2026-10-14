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

# Code generation and Metal syntax/static assertions only; no GPU device or pipeline.
set -euo pipefail
if [[ $# -lt 1 || $# -gt 2 ]]; then
    echo "Usage: $0 BUILD [OUTPUT]" >&2
    exit 1
fi
build=$(cd "$1" && pwd)
output=${2:-"$build/composite-metal"}
mkdir -p "$output"
"$build/spirv-cross-msl-capture-composites-test" "$build/msl_capture_composites.vert.spv" "$build/msl_capture_composites.frag.spv" "$build/msl_capture_mixed_components.spv" "$build/msl_capture_composites_spec_array.spv" "$build/msl_capture_composites_typed.vert.spv" "$build/msl_capture_composites_typed.frag.spv" "$output" > "$output/api.log" 2>&1 || { cat "$output/api.log"; exit 1; }
: > "$output/syntax.log"
for mode in composites composites-compute composites-native composites-native-compute; do
    for kind in capture replay raster fragment mixed-capture mixed-replay typed-capture typed-replay typed-raster typed-fragment; do
        xcrun metal -x metal -std=macos-metal2.4 -fsyntax-only -fno-modules "$output/$mode-$kind.metal" >> "$output/syntax.log" 2>&1 || { cat "$output/syntax.log"; exit 1; }
    done
done
printf 'API checks and 40 Metal 2.4 syntax/static-assertion checks passed (CPU only).\n'
