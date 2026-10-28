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

struct Nested
{
    vec2 value;
    float weights[2];
};

layout(location = 0) pervertexEXT in vec2 texcoord[3][2];
layout(location = 2) pervertexEXT in mat2 transforms[3];
layout(location = 4) pervertexEXT in Inputs
{
    vec3 color;
    mat2 basis;
    Nested nested;
} inputs[3];
layout(location = 10, component = 0) pervertexEXT in ivec2 low[3];
layout(location = 10, component = 2) pervertexEXT in ivec2 high[3];
layout(location = 11) pervertexEXT in uint first[1];
layout(location = 12) pervertexEXT in float pair[2];
layout(location = 13) pervertexEXT in mat2 matrixArray[3][2];
layout(location = 17) pervertexEXT in vec2 nestedArray[3][2][2];
pervertexEXT in Located
{
    layout(location = 21) vec2 a;
    layout(location = 24) float b;
} located[3];
layout(location = 0) out vec4 outputColor;

vec3 read_block(int index)
{
    return inputs[index].color + vec3(inputs[index].basis[1], inputs[index].nested.weights[index % 2]);
}

void main()
{
    int index = int(gl_FragCoord.x) % 3;
    outputColor = vec4(read_block(index), pair[index % 2]);
    outputColor.xy += texcoord[index][index % 2] + transforms[index][index % 2];
    outputColor.xy += inputs[index].nested.value + vec2(low[index] + high[index]);
    outputColor.w += float(first[0]) + located[index].b;
    outputColor.xy += matrixArray[index][index % 2][0] + nestedArray[index][1][index % 2] + located[index].a;
}
