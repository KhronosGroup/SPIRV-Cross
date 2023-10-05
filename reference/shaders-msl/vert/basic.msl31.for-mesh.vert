#include <metal_stdlib>
#include <simd/simd.h>
#include <metal_mesh>

using namespace metal;

struct UBO
{
    float4x4 uMVP;
};

struct main0_out
{
    float3 vNormal [[user(locn0)]];
    float4 gl_Position [[position]];
};

struct main0_in
{
    float4 aVertex;
    float3 aNormal;
};

main0_out main0(main0_in in, constant UBO& _16)
{
    main0_out out = {};
    out.gl_Position = _16.uMVP * in.aVertex;
    out.vNormal = in.aNormal;
    return out;
}

struct Payload
{
    main0_out vertices[3];
};
struct DrawInfo
{
    int32_t indexed;
    int32_t indexSize;
    int64_t indexBuffer;
};
[[object]]  void main0(object_data Payload &payload [[payload]], mesh_grid_properties meshGridProperties, constant DrawInfo *drawInfo [[buffer(20)]],
device uchar *vb29 [[buffer(29)]],
device uchar *vb30 [[buffer(30)]],
constant UBO& _16 [[buffer(0)]],
uint3 positionInGrid [[thread_position_in_grid]])
{
    int startingIndex = positionInGrid.x * 3;
    int vertexCount = 3;
    int instanceIndex = positionInGrid.y;
    for (int i = 0; i < vertexCount; ++i)
    {
        uint vertexIndex;
        if (drawInfo->indexed)
        {
            
		if (drawInfo->indexSize == 1) {
			vertexIndex = ((constant uchar *)drawInfo->indexBuffer)[startingIndex + i];
			if (vertexIndex == 0xff) {
				return;
			}
		} else if (drawInfo->indexSize == 2) {
			vertexIndex = ((constant ushort *)drawInfo->indexBuffer)[startingIndex + i];
			if (vertexIndex == 0xffff) {
				return;
			}
		} else {
			vertexIndex = ((constant uint *)drawInfo->indexBuffer)[startingIndex + i];
			if (vertexIndex == 0xffffffff) {
				return;
			}
		}
        }
        else vertexIndex = startingIndex + i;
        main0_in in;
        in.aVertex = *(device packed_float4 *)(vb30 + 0 + vertexIndex * 16);
        in.aNormal = *(device packed_float3 *)(vb29 + 0 + vertexIndex * 12);
        payload.vertices[i] = main0(
        in
        , _16
        );
    }
    meshGridProperties.set_threadgroups_per_grid(uint3(1, 1, 1));
}
