#pragma clang diagnostic ignored "-Wmissing-prototypes"

#include <metal_stdlib>
#include <simd/simd.h>
#if __METAL_VERSION__ >= 230
#include <metal_raytracing>
using namespace metal::raytracing;
#endif

using namespace metal;

struct Registers
{
    uint value;
};

static inline __attribute__((always_inline))
uint readPushConstant(SPV_RAY_CONTEXT_ARGS(float))
{
    return spvPushConstant<Registers>(spvRayState).value;
}

[[visible]] void main0(SPV_RAY_CONTEXT_ARGS(float))
{
    spvRayData<float>(SPV_RAY_INCOMING_DATA) = float(readPushConstant(SPV_RAY_CONTEXT_NAMES));
}

