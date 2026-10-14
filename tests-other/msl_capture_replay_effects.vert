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

layout(set = 0, binding = 0, std430) buffer AppSideEffects { uint counter; } appSideEffects;
layout(location = 0) noperspective out vec3 color;
layout(location = 0, component = 3) noperspective out float alpha;
layout(location = 2) flat out uint spvReplayPrimitive;
void app_write_side_effect()
{
    atomicAdd(appSideEffects.counter, 1u);
}
void main()
{
    app_write_side_effect();
    color = vec3(gl_VertexIndex, gl_InstanceIndex, 1);
    alpha = 0.5;
    spvReplayPrimitive = 7u;
    gl_Position = vec4(color, 1);
}
