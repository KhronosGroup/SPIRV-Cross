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

layout(location = 0) pervertexEXT in vec4 value[3];
layout(location = 0) out vec4 outputColor;

void main()
{
    outputColor = value[0] + value[1] * 2.0 + value[2] * 4.0;
}
