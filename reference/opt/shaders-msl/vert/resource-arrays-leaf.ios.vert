#pragma clang diagnostic ignored "-Wmissing-prototypes"
#pragma clang diagnostic ignored "-Wmissing-braces"

#include <metal_stdlib>
#include <simd/simd.h>

using namespace metal;

template<typename T, size_t Num>
struct spvUnsafeArray
{
    T elements[Num ? Num : 1];
    
    thread T& operator [] (size_t pos) thread
    {
        return elements[pos];
    }
    constexpr const thread T& operator [] (size_t pos) const thread
    {
        return elements[pos];
    }
    
    device T& operator [] (size_t pos) device
    {
        return elements[pos];
    }
    constexpr const device T& operator [] (size_t pos) const device
    {
        return elements[pos];
    }
    
    constexpr const constant T& operator [] (size_t pos) const constant
    {
        return elements[pos];
    }
    
    threadgroup T& operator [] (size_t pos) threadgroup
    {
        return elements[pos];
    }
    constexpr const threadgroup T& operator [] (size_t pos) const threadgroup
    {
        return elements[pos];
    }
};

struct storage_block
{
    uint4 baz;
    int2 quux;
};

struct constant_block
{
    float4 foo;
    int bar;
};

#ifndef SPIRV_CROSS_CONSTANT_ID_0
#define SPIRV_CROSS_CONSTANT_ID_0 3
#endif
constant int arraySize = SPIRV_CROSS_CONSTANT_ID_0;

vertex void main0(device storage_block* storage_0 [[buffer(0)]], device storage_block* storage_1 [[buffer(1)]], constant constant_block* constants_0 [[buffer(2)]], constant constant_block* constants_1 [[buffer(3)]], constant constant_block* constants_2 [[buffer(4)]], constant constant_block* constants_3 [[buffer(5)]], array<texture2d<int>, 3> images [[texture(0)]])
{
    spvUnsafeArray<device storage_block*, 2> storage =
    {
        storage_0,
        storage_1,
    };

    spvUnsafeArray<constant constant_block*, 4> constants =
    {
        constants_0,
        constants_1,
        constants_2,
        constants_3,
    };

    storage[0]->baz = uint4(constants[3]->foo);
    storage[1]->quux = images[2].read(uint2(int2(constants[1]->bar))).xy;
}

