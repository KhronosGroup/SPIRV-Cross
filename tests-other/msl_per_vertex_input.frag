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

layout(location = 0) pervertexEXT in vec3 color[3];
layout(location = 1, component = 2) pervertexEXT in vec2 uv[3];
layout(location = 2) pervertexEXT in ivec4 offsets[3];
layout(location = 3) pervertexEXT in uvec3 labels[3];
layout(location = 4) pervertexEXT in f16vec2 smallColor[3];
layout(location = 5) pervertexEXT in i16vec3 smallOffsets[3];
layout(location = 6) pervertexEXT in u16vec4 smallLabels[3];
layout(location = 7, component = 3) pervertexEXT in float weight[3];
layout(location = 8) in vec2 ordinary;
layout(set = 0, binding = 0, std430) readonly buffer Values { float value; } values[2];
layout(location = 0) out vec4 outputColor;

vec3 read_color(int index)
{
    return color[index];
}

void main()
{
    int index = int(gl_FragCoord.x) % 3;
    outputColor = vec4(read_color(index), weight[index]);
    outputColor += vec4(uv[index], ordinary) + vec4(offsets[index]);
    outputColor.xyz += vec3(labels[index]) + vec3(smallOffsets[index]);
    outputColor.xy += vec2(smallColor[index]);
    outputColor += vec4(smallLabels[index]) + vec4(values[0].value + values[1].value);
}
