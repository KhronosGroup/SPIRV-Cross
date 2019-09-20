#pragma clang diagnostic ignored "-Wmissing-prototypes"
#pragma clang diagnostic ignored "-Wmissing-braces"
#pragma clang diagnostic ignored "-Wunused-variable"

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

struct SSBO
{
    spvUnsafeArray<char, 16> i8;
    spvUnsafeArray<uchar, 16> u8;
};

struct Push
{
    char i8;
    uchar u8;
};

struct UBO
{
    char i8;
    uchar u8;
};

struct main0_out
{
    int4 FragColorInt [[color(0)]];
    uint4 FragColorUint [[color(1)]];
};

struct main0_in
{
    int4 vColor [[user(locn0)]];
};

fragment main0_out main0(main0_in in [[stage_in]], device SSBO& ssbo [[buffer(0)]], constant Push& registers [[buffer(1)]], constant UBO& ubo [[buffer(2)]])
{
    main0_out out = {};
    short _196 = short(10);
    int _197 = 20;
    char2 _198 = as_type<char2>(_196);
    char4 _199 = as_type<char4>(_197);
    _196 = as_type<short>(_198);
    _197 = as_type<int>(_199);
    ssbo.i8[0] = _199.x;
    ssbo.i8[1] = _199.y;
    ssbo.i8[2] = _199.z;
    ssbo.i8[3] = _199.w;
    ushort _220 = ushort(10);
    uint _221 = 20u;
    uchar2 _222 = as_type<uchar2>(_220);
    uchar4 _223 = as_type<uchar4>(_221);
    _220 = as_type<ushort>(_222);
    _221 = as_type<uint>(_223);
    ssbo.u8[0] = _223.x;
    ssbo.u8[1] = _223.y;
    ssbo.u8[2] = _223.z;
    ssbo.u8[3] = _223.w;
    char4 _246 = char4(in.vColor);
    char4 _244 = _246;
    _244 += char4(registers.i8);
    _244 += char4(-40);
    _244 += char4(-50);
    _244 += char4(char(10), char(20), char(30), char(40));
    _244 += char4(ssbo.i8[4]);
    _244 += char4(ubo.i8);
    out.FragColorInt = int4(_244);
    uchar4 _271 = uchar4(_246);
    _271 += uchar4(registers.u8);
    _271 += uchar4(216);
    _271 += uchar4(206);
    _271 += uchar4(uchar(10), uchar(20), uchar(30), uchar(40));
    _271 += uchar4(ssbo.u8[4]);
    _271 += uchar4(ubo.u8);
    out.FragColorUint = uint4(_271);
    return out;
}

