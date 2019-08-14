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

struct main0_out
{
    float FragColor [[color(0)]];
};

struct main0_in
{
    float vColor [[user(locn0)]];
};

#line 6 "test.frag"
static inline __attribute__((always_inline))
void func(thread float& FragColor, thread float& vColor)
{
#line 8 "test.frag"
    FragColor = 1.0;
#line 9 "test.frag"
    FragColor = 2.0;
#line 10 "test.frag"
    if (vColor < 0.0)
    {
#line 12 "test.frag"
        FragColor = 3.0;
    }
    else
    {
#line 16 "test.frag"
        FragColor = 4.0;
    }
#line 19 "test.frag"
    for (int i = 0; float(i) < (40.0 + vColor); i += (int(vColor) + 5))
    {
#line 21 "test.frag"
        FragColor += 0.20000000298023223876953125;
#line 22 "test.frag"
        FragColor += 0.300000011920928955078125;
    }
#line 25 "test.frag"
    switch (int(vColor))
    {
        case 0:
        {
#line 28 "test.frag"
            FragColor += 0.20000000298023223876953125;
#line 29 "test.frag"
            break;
        }
        case 1:
        {
#line 32 "test.frag"
            FragColor += 0.4000000059604644775390625;
#line 33 "test.frag"
            break;
        }
        default:
        {
#line 36 "test.frag"
            FragColor += 0.800000011920928955078125;
#line 37 "test.frag"
            break;
        }
    }
    do
    {
#line 42 "test.frag"
        FragColor += (10.0 + vColor);
    } while (FragColor < 100.0);
}

#line 46 "test.frag"
fragment main0_out main0(main0_in in [[stage_in]])
{
    main0_out out = {};
#line 48 "test.frag"
    func(out.FragColor, in.vColor);
    return out;
}

