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

layout(constant_id = 0) const int N = 2;
layout(location = 0, component = 2) out vec2 hi[N][2];
layout(location = 0, component = 0) out vec2 lo[N][2];
void main()
{
    for (int i = 0; i < N; i++)
        for (int j = 0; j < 2; j++)
        {
            hi[i][j] = vec2(i, j);
            lo[i][j] = vec2(j, i);
        }
    gl_Position = vec4(0, 0, 0, 1);
}
