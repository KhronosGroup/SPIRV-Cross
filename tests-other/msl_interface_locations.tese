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

layout(triangles, equal_spacing, cw) in;

in B
{
    layout(location = 0, component = 0) vec2 lo[2];
    layout(location = 0, component = 2) vec2 hi[2];
} b[];

void main()
{
    gl_Position = vec4(b[0].lo[0] + b[0].lo[1], b[0].hi[0] + b[0].hi[1]);
}
