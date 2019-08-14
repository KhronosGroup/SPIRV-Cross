#pragma clang diagnostic ignored "-Wmissing-prototypes"
#pragma clang diagnostic ignored "-Wmissing-braces"
#pragma clang diagnostic ignored "-Wunused-variable"

#include <metal_stdlib>
#include <simd/simd.h>
	
template <typename T, size_t Num>
struct unsafe_array
{
	T __Elements[Num ? Num : 1];
	
	constexpr size_t size() const thread { return Num; }
	constexpr size_t max_size() const thread { return Num; }
	constexpr bool empty() const thread { return Num == 0; }
	
	constexpr size_t size() const device { return Num; }
	constexpr size_t max_size() const device { return Num; }
	constexpr bool empty() const device { return Num == 0; }
	
	constexpr size_t size() const constant { return Num; }
	constexpr size_t max_size() const constant { return Num; }
	constexpr bool empty() const constant { return Num == 0; }
	
	constexpr size_t size() const threadgroup { return Num; }
	constexpr size_t max_size() const threadgroup { return Num; }
	constexpr bool empty() const threadgroup { return Num == 0; }
	
	thread T &operator[](size_t pos) thread
	{
		return __Elements[pos];
	}
	constexpr const thread T &operator[](size_t pos) const thread
	{
		return __Elements[pos];
	}
	
	device T &operator[](size_t pos) device
	{
		return __Elements[pos];
	}
	constexpr const device T &operator[](size_t pos) const device
	{
		return __Elements[pos];
	}
	
	constexpr const constant T &operator[](size_t pos) const constant
	{
		return __Elements[pos];
	}
	
	threadgroup T &operator[](size_t pos) threadgroup
	{
		return __Elements[pos];
	}
	constexpr const threadgroup T &operator[](size_t pos) const threadgroup
	{
		return __Elements[pos];
	}
};

using namespace metal;

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

static inline __attribute__((always_inline))
void doWork(thread unsafe_array<device storage_block*,2> (&storage), thread unsafe_array<constant constant_block*,4> (&constants), thread const array<texture2d<int>, 3> images)
{
    storage[0]->baz = uint4(constants[3]->foo);
    storage[1]->quux = images[2].read(uint2(int2(constants[1]->bar))).xy;
}

vertex void main0(device storage_block* storage_0 [[buffer(0)]], device storage_block* storage_1 [[buffer(1)]], constant constant_block* constants_0 [[buffer(2)]], constant constant_block* constants_1 [[buffer(3)]], constant constant_block* constants_2 [[buffer(4)]], constant constant_block* constants_3 [[buffer(5)]], array<texture2d<int>, 3> images [[texture(0)]])
{
    unsafe_array<device storage_block*,2> storage =
    {
        storage_0,
        storage_1,
    };

    unsafe_array<constant constant_block*,4> constants =
    {
        constants_0,
        constants_1,
        constants_2,
        constants_3,
    };

    doWork(storage, constants, images);
}

