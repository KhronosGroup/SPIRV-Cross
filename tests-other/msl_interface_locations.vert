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
out Located
{
    layout(location = 4) Inner nested;
    layout(location = 10) mat2 basis;
    layout(location = 15) float tail;
    layout(location = 18, component = 2) vec2 upper[2];
    layout(location = 18, component = 0) vec2 lower[2];
    layout(location = 2, component = 1) float early;
    layout(location = 23) Inner structured[2];
} located;

void main()
{
    located.nested = Inner(vec3(1), float[2](2, 3));
    located.basis = mat2(1);
    located.tail = 4;
    located.upper = vec2[2](vec2(5), vec2(6));
    located.lower = vec2[2](vec2(7), vec2(8));
    located.early = 9;
    located.structured = Inner[2](located.nested, located.nested);
    gl_Position = vec4(0, 0, 0, 1);
}
