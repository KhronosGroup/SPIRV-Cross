#pragma clang diagnostic ignored "-Wmissing-prototypes"

#include <metal_stdlib>
#include <simd/simd.h>
#if __METAL_VERSION__ >= 230
#include <metal_raytracing>
using namespace metal::raytracing;
#endif

using namespace metal;

template<typename T, uint N>
inline void spvArrayCopyFromConstantToStack(thread T (&dst)[N], constant T (&src)[N])
{
    for (uint i = 0; i < N; i++)
    {
        dst[i] = src[i];
    }
}

template<typename T, uint N>
inline void spvArrayCopyFromConstantToThreadGroup(threadgroup T (&dst)[N], constant T (&src)[N])
{
    for (uint i = 0; i < N; i++)
    {
        dst[i] = src[i];
    }
}

template<typename T, uint N>
inline void spvArrayCopyFromStackToStack(thread T (&dst)[N], thread const T (&src)[N])
{
    for (uint i = 0; i < N; i++)
    {
        dst[i] = src[i];
    }
}

template<typename T, uint N>
inline void spvArrayCopyFromStackToThreadGroup(threadgroup T (&dst)[N], thread const T (&src)[N])
{
    for (uint i = 0; i < N; i++)
    {
        dst[i] = src[i];
    }
}

template<typename T, uint N>
inline void spvArrayCopyFromThreadGroupToStack(thread T (&dst)[N], threadgroup const T (&src)[N])
{
    for (uint i = 0; i < N; i++)
    {
        dst[i] = src[i];
    }
}

template<typename T, uint N>
inline void spvArrayCopyFromThreadGroupToThreadGroup(threadgroup T (&dst)[N], threadgroup const T (&src)[N])
{
    for (uint i = 0; i < N; i++)
    {
        dst[i] = src[i];
    }
}

template<typename T, uint N>
inline void spvArrayCopyFromDeviceToDevice(device T (&dst)[N], device const T (&src)[N])
{
    for (uint i = 0; i < N; i++)
    {
        dst[i] = src[i];
    }
}

template<typename T, uint N>
inline void spvArrayCopyFromConstantToDevice(device T (&dst)[N], constant T (&src)[N])
{
    for (uint i = 0; i < N; i++)
    {
        dst[i] = src[i];
    }
}

template<typename T, uint N>
inline void spvArrayCopyFromStackToDevice(device T (&dst)[N], thread const T (&src)[N])
{
    for (uint i = 0; i < N; i++)
    {
        dst[i] = src[i];
    }
}

template<typename T, uint N>
inline void spvArrayCopyFromThreadGroupToDevice(device T (&dst)[N], threadgroup const T (&src)[N])
{
    for (uint i = 0; i < N; i++)
    {
        dst[i] = src[i];
    }
}

template<typename T, uint N>
inline void spvArrayCopyFromDeviceToStack(thread T (&dst)[N], device const T (&src)[N])
{
    for (uint i = 0; i < N; i++)
    {
        dst[i] = src[i];
    }
}

template<typename T, uint N>
inline void spvArrayCopyFromDeviceToThreadGroup(threadgroup T (&dst)[N], device const T (&src)[N])
{
    for (uint i = 0; i < N; i++)
    {
        dst[i] = src[i];
    }
}

static inline __attribute__((always_inline))
void _15(thread device const ulong* (&spvReturnValue)[2], SPV_RAY_CONTEXT_ARGS(void))
{
    device const ulong* _20[2] = { spvAccelerationStructureFromAddress(as_type<ulong>(uint2(0u)), reinterpret_cast<device const ulong2*>(spvRuntimeBuffer<3>(spvRayState))), spvAccelerationStructureFromAddress(as_type<ulong>(uint2(2u, 0u)), reinterpret_cast<device const ulong2*>(spvRuntimeBuffer<3>(spvRayState))) };
    spvArrayCopyFromStackToStack(spvReturnValue, _20);
}

[[visible]] void main0(SPV_RAY_CONTEXT_ARGS(void))
{
    device const ulong* _14[2];
    _15(_14, SPV_RAY_CONTEXT_NAMES);
}

