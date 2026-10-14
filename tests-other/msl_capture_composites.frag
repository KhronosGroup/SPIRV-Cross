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
struct Leaf { vec3 color; float weights[2]; };
struct Node { Leaf branches[2][2]; mat3x2 matrices[2][2]; };
layout(location=4) pervertexEXT in Node nested[3];
layout(location=32) pervertexEXT in mat2x3 basis[3][2][2];
layout(location=42, component=2) pervertexEXT in vec2 upper[3][2][2];
layout(location=42, component=0) pervertexEXT in vec2 lower[3][2][2];
layout(location=2, component=1) pervertexEXT in float early[3];
layout(location=50) pervertexEXT in Leaf structs[3][1][2];
layout(location=60) pervertexEXT in mat2x3 tops[3][2][2];
layout(location=70, component=2) pervertexEXT in vec2 topHi[3][2][2];
layout(location=70, component=0) pervertexEXT in vec2 topLo[3][2][2];
layout(location=80) pervertexEXT in Node topNodes[3][1][1];
layout(location=0) out vec4 color;
void main() {
 int v=int(gl_FragCoord.x)%3;
 color=vec4(nested[v].branches[1][1].color + basis[v][1][1][1] + tops[v][1][1][1] + structs[v][0][1].color + topNodes[v][0][0].branches[1][1].color, early[v]);
 color.xy+=upper[v][1][1]+lower[v][1][1]+topHi[v][1][1]+topLo[v][1][1]+nested[v].matrices[1][1][2];
}
