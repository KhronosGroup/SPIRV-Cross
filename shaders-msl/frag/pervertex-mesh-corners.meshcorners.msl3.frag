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
// Reads the PerVertexKHR corners (Locations 0 and 3) that a mesh shader copies per primitive.
#extension GL_EXT_fragment_shader_barycentric : require
layout(location = 0) pervertexEXT in vec4 corner[3];
layout(location = 2) in vec4 interpolated;
layout(location = 3, component = 1) pervertexEXT in vec2 packed[3];
layout(location = 0) out vec4 color;

void main()
{
	color = gl_BaryCoordEXT.x * corner[0] + gl_BaryCoordEXT.y * corner[1] + gl_BaryCoordEXT.z * corner[2] +
	        vec4(packed[1], packed[2]) + interpolated;
}
