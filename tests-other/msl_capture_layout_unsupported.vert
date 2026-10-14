#version 450
// Copyright 2026 Jean-Philippe Meunier
// SPDX-License-Identifier: Apache-2.0
//
// Licensed under the Apache License, Version 2.0 (the License);
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an AS IS BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#extension GL_ARB_gpu_shader_int64 : require
layout(location = 0) flat out uint64_t wide;
layout(location = 1) out mat2 matrixOutput;
void main()
{
    wide = uint64_t(1);
    matrixOutput = mat2(1);
    gl_Position = vec4(0, 0, 0, 1);
}
