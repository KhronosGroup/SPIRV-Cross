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

struct Leaf { vec3 color; float weights[2]; };
struct Node { Leaf branches[2][2]; mat3x2 matrices[2][2]; };
out Block {
 layout(location=4) Node nested;
 layout(location=32) mat2x3 basis[2][2];
 layout(location=42, component=2) vec2 upper[2][2];
 layout(location=42, component=0) vec2 lower[2][2];
 layout(location=2, component=1) float early;
 layout(location=50) Leaf structs[1][2];
} located;
layout(location=60) out mat2x3 tops[2][2];
layout(location=70, component=2) out vec2 topHi[2][2];
layout(location=70, component=0) out vec2 topLo[2][2];
layout(location=80) out Node topNodes[1][1];
void fill() {
 for (int i=0;i<2;i++) for(int j=0;j<2;j++) {
  located.nested.branches[i][j] = Leaf(vec3(i+j),float[2](1,2));
  located.nested.matrices[i][j] = mat3x2(1);
  located.basis[i][j]=mat2x3(1);
  located.upper[i][j]=vec2(3);
  located.lower[i][j]=vec2(4);
  tops[i][j]=mat2x3(2);
  topHi[i][j]=vec2(5);
  topLo[i][j]=vec2(6);
 }
 located.structs[0][0]=located.nested.branches[0][0];
 located.structs[0][1]=located.nested.branches[1][1];
 located.early=7;
 topNodes[0][0]=located.nested;
}
void main() { fill(); gl_Position=vec4(0,0,0,1); }
