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

#extension GL_EXT_fragment_shader_barycentric : require
#extension GL_EXT_shader_explicit_arithmetic_types_float16 : require
#extension GL_EXT_shader_explicit_arithmetic_types_int16 : require
#extension GL_EXT_shader_16bit_storage : require

layout(location = 0) pervertexEXT in f16vec3 color[3];
layout(location = 1) pervertexEXT in i16vec2 offsets[3];
layout(location = 2) pervertexEXT in uint16_t labels[3];
layout(location = 0) out vec4 outputColor;

void main()
{
    int index = int(gl_FragCoord.x) % 3;
    outputColor = vec4(vec3(color[index]), float(labels[index]));
    outputColor.xy += vec2(offsets[index]);
}
