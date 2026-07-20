#version 460
#extension GL_EXT_ray_tracing : require

layout(set = 0, binding = 0) uniform accelerationStructureEXT scene;
layout(location = 0) rayPayloadInEXT vec4 incomingPayload[2][3];
layout(location = 1) rayPayloadEXT float nestedPayload[2][3];

void updatePayload(inout vec4 payload[2][3])
{
    payload[1][2].x += 1.0;
}

void main()
{
    updatePayload(incomingPayload);
    nestedPayload[1][2] = incomingPayload[1][2].x;
    traceRayEXT(scene, 0u, 255u, 0u, 0u, 0u, vec3(0.0), 0.0, vec3(0.0, 0.0, -1.0), 1.0, 1);
    incomingPayload[1][2].x = nestedPayload[1][2];
}
