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

struct Foo
{
    short b1;
    short2 b2;
    spvUnsafeArray<short, 2> b3;
    spvUnsafeArray<short2, 2> b4;
};

constant Foo _20 = {};
constant bool CONST_BOOL_tmp [[function_constant(0)]];
constant bool CONST_BOOL = is_function_constant_defined(CONST_BOOL_tmp) ? CONST_BOOL_tmp : false;
constant bool2 _21 = bool2(CONST_BOOL, CONST_BOOL);
constant bool2 _29 = bool2(CONST_BOOL, CONST_BOOL);
constant bool2 _30 = bool2(CONST_BOOL, CONST_BOOL);

vertex void main0()
{
    Foo foo;
    foo.b1 = short(CONST_BOOL);
    foo.b2 = short2(_21);
    spvUnsafeArray<bool, 2> _25 = spvUnsafeArray<bool, 2>({ CONST_BOOL, CONST_BOOL });
    foo.b3 = { short(_25[0]), short(_25[1]) };
    spvUnsafeArray<bool2, 2> _31 = spvUnsafeArray<bool2, 2>({ _29, _30 });
    foo.b4 = { short2(_31[0]), short2(_31[1]) };
    bool b1 = bool(foo.b1);
    bool2 b2 = bool2(foo.b2);
    spvUnsafeArray<bool, 2> b3 = { bool(foo.b3[0]), bool(foo.b3[1]) };
    spvUnsafeArray<bool2, 2> b4 = { bool2(foo.b4[0]), bool2(foo.b4[1]) };
    Foo _47;
    _47.b1 = short(CONST_BOOL);
    _47.b2 = short2(_29);
    _47.b3 = { short(_25[0]), short(_25[1]) };
    _47.b4 = { short2(_31[0]), short2(_31[1]) };
    b1 = bool(_47.b1);
    b2 = bool2(_47.b2);
    b3 = { bool(_47.b3[0]), bool(_47.b3[1]) };
    b4 = { bool2(_47.b4[0]), bool2(_47.b4[1]) };
}

