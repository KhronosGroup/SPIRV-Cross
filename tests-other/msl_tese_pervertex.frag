#version 450
// Copyright 2026 Jean-Philippe Meunier
// SPDX-License-Identifier: Apache-2.0
#extension GL_EXT_fragment_shader_barycentric : require
layout(location = 2) pervertexEXT in vec3 domain[3];
layout(location = 0) out vec4 color;
void main()
{
    bool ordered = all(lessThan(abs(domain[0] - vec3(1, 0, 0)), vec3(0.001))) &&
                   all(lessThan(abs(domain[1] - vec3(0, 1, 0)), vec3(0.001))) &&
                   all(lessThan(abs(domain[2] - vec3(0, 0, 1)), vec3(0.001)));
    color = ordered ? vec4(1, 0, 0, 1) : vec4(0, 1, 0, 1);
}
