#pragma clang diagnostic ignored "-Wmissing-prototypes"
#pragma clang diagnostic ignored "-Wmissing-braces"
#pragma clang diagnostic ignored "-Wunused-variable"

#include <metal_stdlib>
#include <simd/simd.h>
#include <metal_atomic>
	
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

struct type_StructuredBuffer_v4float
{
    unsafe_array<float4,1> _m0;
};

struct type_Globals
{
    uint2 ShadowTileListGroupSize;
};

constant float3 _70 = {};

struct main0_out
{
    float4 out_var_SV_Target0 [[color(0)]];
};

struct main0_in
{
    uint in_var_TEXCOORD0 [[user(locn0)]];
};

// Returns buffer coords clamped to storage buffer size
#define spvStorageBufferCoords(idx, sizes, type, coord) metal::min(coord, (sizes[idx*2] / sizeof(type)) - 1)
fragment main0_out main0(main0_in in [[stage_in]], const device type_StructuredBuffer_v4float& CulledObjectBoxBounds [[buffer(1)]], constant type_Globals& _Globals [[buffer(2)]], texture2d<uint> RWShadowTileNumCulledObjects [[texture(0)]], device atomic_uint* RWShadowTileNumCulledObjects_atomic [[buffer(0)]], float4 gl_FragCoord [[position]])
{
    main0_out out = {};
    uint2 _77 = uint2(gl_FragCoord.xy);
    uint _78 = _77.y;
    uint _83 = _77.x;
    float2 _91 = float2(float(_83), float((_Globals.ShadowTileListGroupSize.y - 1u) - _78));
    float2 _93 = float2(_Globals.ShadowTileListGroupSize);
    float2 _96 = ((_91 / _93) * float2(2.0)) - float2(1.0);
    float2 _100 = (((_91 + float2(1.0)) / _93) * float2(2.0)) - float2(1.0);
    float3 _102 = float3(_100.x, _100.y, _70.z);
    _102.z = 1.0;
    uint _103 = in.in_var_TEXCOORD0 * 5u;
    uint _107 = _103 + 1u;
    if (all(CulledObjectBoxBounds._m0[_107].xy > _96.xy) && all(CulledObjectBoxBounds._m0[_103].xyz < _102))
    {
        float _122 = _96.x;
        float _123 = _96.y;
        unsafe_array<float3,8> _73;
        _73[0] = float3(_122, _123, -1000.0);
        float _126 = _100.x;
        _73[1] = float3(_126, _123, -1000.0);
        float _129 = _100.y;
        _73[2] = float3(_122, _129, -1000.0);
        _73[3] = float3(_126, _129, -1000.0);
        _73[4] = float3(_122, _123, 1.0);
        _73[5] = float3(_126, _123, 1.0);
        _73[6] = float3(_122, _129, 1.0);
        _73[7] = float3(_126, _129, 1.0);
        float3 _155;
        float3 _158;
        _155 = float3(-500000.0);
        _158 = float3(500000.0);
        for (int _160 = 0; _160 < 8; )
        {
            float3 _166 = _73[_160] - (float3(0.5) * (CulledObjectBoxBounds._m0[_103].xyz + CulledObjectBoxBounds._m0[_107].xyz));
            float3 _170 = float3(dot(_166, CulledObjectBoxBounds._m0[_103 + 2u].xyz), dot(_166, CulledObjectBoxBounds._m0[_103 + 3u].xyz), dot(_166, CulledObjectBoxBounds._m0[_103 + 4u].xyz));
            _155 = fast::max(_155, _170);
            _158 = fast::min(_158, _170);
            _160++;
            continue;
        }
        if (all(_158 < float3(1.0)) && all(_155 > float3(-1.0)))
        {
            uint _179 = atomic_fetch_add_explicit((volatile device atomic_uint*)&RWShadowTileNumCulledObjects_atomic[(_78 * _Globals.ShadowTileListGroupSize.x) + _83], 1u, memory_order_relaxed);
        }
    }
    out.out_var_SV_Target0 = float4(0.0);
    return out;
}

