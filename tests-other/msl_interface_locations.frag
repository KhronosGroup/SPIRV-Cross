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


struct Inner { vec3 color; float weights[2]; };
in Located
{
    layout(location = 4) Inner nested;
    layout(location = 10) mat2 basis;
    layout(location = 15) float tail;
    layout(location = 18, component = 2) vec2 upper[2];
    layout(location = 18, component = 0) vec2 lower[2];
    layout(location = 2, component = 1) float early;
    layout(location = 23) Inner structured[2];
} located;
layout(location = 0) out vec4 color;

void main()
{
    color = vec4(located.nested.color, located.tail + located.early);
    color += vec4(located.nested.weights[0], located.nested.weights[1], located.basis[0]);
    color += vec4(located.basis[1], located.upper[0] + located.upper[1] + located.lower[0] + located.lower[1]);
    for (int i = 0; i < 2; i++)
        color += vec4(located.structured[i].color, located.structured[i].weights[0] + located.structured[i].weights[1]);
}
