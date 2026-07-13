#version 460
#extension GL_EXT_ray_tracing : require
#extension GL_EXT_ray_cull_mask : require

layout(location = 0) rayPayloadInEXT vec4 payload;
hitAttributeEXT vec2 hitData;
layout(shaderRecordEXT, std430) buffer Record
{
    float factor;
} record;

void main()
{
    float builtins = dot(gl_WorldRayOriginEXT + gl_WorldRayDirectionEXT + gl_ObjectRayOriginEXT +
                         gl_ObjectRayDirectionEXT, vec3(1.0));
    builtins += gl_RayTminEXT + gl_RayTmaxEXT + float(gl_InstanceCustomIndexEXT) + float(gl_InstanceID) +
                float(gl_PrimitiveID) + float(gl_GeometryIndexEXT) + float(gl_HitKindEXT) +
                float(gl_IncomingRayFlagsEXT) + float(gl_CullMaskEXT) + gl_ObjectToWorldEXT[0][0] +
                gl_WorldToObjectEXT[0][0] + record.factor;
    payload = vec4(hitData, builtins, 1.0);
}
