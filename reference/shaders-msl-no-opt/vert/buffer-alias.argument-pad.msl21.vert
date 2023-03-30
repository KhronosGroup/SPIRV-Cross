#include <metal_stdlib>
#include <simd/simd.h>

using namespace metal;

struct Buf16
{
    ushort u16s[1];
};

struct Buf32
{
    uint u32s[1];
};

struct Float32
{
    float f32s[1];
};

struct spvDescriptorSetBuffer0
{
    device Buf16* m_13 [[id(0)]];
};

struct main0_out
{
    uint u16o [[user(locn0)]];
    uint u32o [[user(locn1)]];
    float f32o [[user(locn2)]];
    float4 gl_Position [[position]];
};

vertex main0_out main0(constant spvDescriptorSetBuffer0& spvDescriptorSet0 [[buffer(0)]])
{
    device auto& _24 = (device Buf32&)(*spvDescriptorSet0.m_13);
    device auto& _35 = (device Float32&)(*spvDescriptorSet0.m_13);
    main0_out out = {};
    out.u16o = uint((*spvDescriptorSet0.m_13).u16s[0]);
    out.u32o = _24.u32s[2];
    out.f32o = _35.f32s[3];
    out.gl_Position = float4(0.0);
    return out;
}

