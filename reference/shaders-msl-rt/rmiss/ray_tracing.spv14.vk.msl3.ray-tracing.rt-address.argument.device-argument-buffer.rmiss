#include <metal_stdlib>
#include <simd/simd.h>
#if __METAL_VERSION__ >= 230
#include <metal_raytracing>
using namespace metal::raytracing;
#endif

using namespace metal;

[[visible]] void main0(SPV_RAY_CONTEXT_ARGS(float))
{
    spvRayData<float>(SPV_RAY_INCOMING_DATA) = 0.0;
}

