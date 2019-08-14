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

struct main0_out
{
    float FragColor [[color(0)]];
};

struct main0_in
{
    float vColor [[user(locn0)]];
};

#line 8 "test.frag"
fragment main0_out main0(main0_in in [[stage_in]])
{
    main0_out out = {};
#line 8 "test.frag"
    out.FragColor = 1.0;
#line 9 "test.frag"
    out.FragColor = 2.0;
#line 10 "test.frag"
    if (in.vColor < 0.0)
    {
#line 12 "test.frag"
        out.FragColor = 3.0;
    }
    else
    {
#line 16 "test.frag"
        out.FragColor = 4.0;
    }
    for (int _126 = 0; float(_126) < (40.0 + in.vColor); )
    {
#line 21 "test.frag"
        out.FragColor += 0.20000000298023223876953125;
#line 22 "test.frag"
        out.FragColor += 0.300000011920928955078125;
        _126 += (int(in.vColor) + 5);
        continue;
    }
    switch (int(in.vColor))
    {
        case 0:
        {
#line 28 "test.frag"
            out.FragColor += 0.20000000298023223876953125;
#line 29 "test.frag"
            break;
        }
        case 1:
        {
#line 32 "test.frag"
            out.FragColor += 0.4000000059604644775390625;
#line 33 "test.frag"
            break;
        }
        default:
        {
#line 36 "test.frag"
            out.FragColor += 0.800000011920928955078125;
#line 37 "test.frag"
            break;
        }
    }
    for (;;)
    {
        out.FragColor += (10.0 + in.vColor);
#line 43 "test.frag"
        if (out.FragColor < 100.0)
        {
        }
        else
        {
            break;
        }
    }
    return out;
}

