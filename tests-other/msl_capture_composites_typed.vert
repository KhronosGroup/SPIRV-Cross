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

#extension GL_EXT_shader_explicit_arithmetic_types_float16 : require
#extension GL_EXT_shader_explicit_arithmetic_types_int16 : require
#extension GL_EXT_shader_16bit_storage : require
struct Leaf { f16vec3 h; int16_t s[2]; uint16_t u; ivec2 i; uvec3 j; f16mat2x3 m[2]; };
layout(location=0) out Leaf typed[2];
void main() { for (int n=0;n<2;n++) typed[n]=Leaf(f16vec3(1),int16_t[2](int16_t(-2),int16_t(3)),uint16_t(4),ivec2(-5),uvec3(6),f16mat2x3[2](f16mat2x3(1),f16mat2x3(2))); gl_Position=vec4(0,0,0,1); }
