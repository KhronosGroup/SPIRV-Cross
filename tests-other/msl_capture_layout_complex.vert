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

#extension GL_ARB_shader_viewport_layer_array : require
#extension GL_EXT_shader_explicit_arithmetic_types_float16 : require
#extension GL_EXT_shader_explicit_arithmetic_types_int16 : require
#extension GL_EXT_shader_16bit_storage : require
layout(location = 7) out vec3 later;
layout(location = 0, component = 2) out vec2 pair;
layout(location = 2) out f16vec3 small;
layout(location = 3) flat out uint16_t label;
layout(location = 4) out Block { float weight; vec3 normal; } blockData;
void write_clip_distances()
{
    gl_ClipDistance[0] = 0;
    gl_ClipDistance[1] = 1;
}

void main()
{
    later = vec3(7);
    pair = vec2(2);
    small = f16vec3(3);
    label = uint16_t(4);
    blockData.weight = 5;
    blockData.normal = vec3(6);
    gl_Position = vec4(0, 0, 0, 1);
    gl_PointSize = 2;
    gl_Layer = 1;
    gl_ViewportIndex = 2;
    write_clip_distances();
}
