// Copyright 2026 Jean-Philippe Meunier
// SPDX-License-Identifier: Apache-2.0
#version 450
#extension GL_EXT_multiview : require
#extension GL_ARB_shader_draw_parameters : require
layout(location = 0) in vec4 position;
layout(location = 1) in vec4 instanceValue;
layout(location = 0) out vec4 value;
layout(location = 1) out vec2 ordinary;
#ifndef VIEW_ONLY
// Locations 2-4 are reserved by the replay primitive key and fragment barycentrics.
layout(location = 5) flat out int appInstance;
layout(location = 6) flat out int appVertex;
#endif
#ifdef EFFECTS
layout(location = 7) flat out int appBaseInstance;
layout(location = 8) flat out int appBaseVertex;
layout(set = 0, binding = 0, std430) buffer AppSideEffects { uint counter; } appSideEffects;
#endif
void main()
{
    gl_Position = position;
#ifdef VIEW_ONLY
    value = vec4(gl_ViewIndex);
    ordinary = vec2(gl_ViewIndex);
#else
    value = instanceValue + vec4(float(gl_ViewIndex), float(gl_InstanceIndex), float(gl_VertexIndex), 0.0);
    ordinary = vec2(gl_ViewIndex, gl_InstanceIndex);
    appInstance = gl_InstanceIndex;
    appVertex = gl_VertexIndex;
#endif
#ifdef EFFECTS
    appBaseInstance = gl_BaseInstanceARB;
    appBaseVertex = gl_BaseVertexARB;
    atomicAdd(appSideEffects.counter, 1u);
    value.w = float(gl_BaseInstanceARB + gl_BaseVertexARB + gl_DrawIDARB);
    gl_ClipDistance[0] = value.x;
#endif
}
