#pragma clang diagnostic ignored "-Wmissing-prototypes"
#pragma clang diagnostic ignored "-Wmissing-braces"

#include <metal_stdlib>
#include <simd/simd.h>
#include <metal_mesh>

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

template<typename V, typename P, int MaxV, int MaxP, metal::topology T>
struct spvMeshStream
{
    using mesh_t = metal::mesh<V, P, MaxV, MaxP, T>;
    thread mesh_t &meshOut;
    int currentVertex = 0;
    int currentIndex = 0;
    int currentVertexInPrimitive = 0;
    int currentPrimitive = 0;
    thread P &primitiveData;
    thread V &vertexData;
    spvMeshStream(thread mesh_t &_meshOut, thread V &_v, thread P &_p) : meshOut(_meshOut), primitiveData(_p), vertexData(_v)
    {
    }
    ~spvMeshStream()
    {
        meshOut.set_primitive_count(currentPrimitive);
    }
    int VperP()
    {
        if (T == metal::topology::triangle)          return 3;
        else if (T == metal::topology::line)         return 2;
        else /* if (T == metal::topology::point) */  return 1;
    }
    void EndPrimitive()
    {
        currentVertexInPrimitive = 0;
    }
    void EmitVertex()
    {
        meshOut.set_vertex(currentVertex++, vertexData);
        currentVertexInPrimitive++;
        if (currentVertexInPrimitive >= VperP())
        {
            if (T == metal::topology::triangle) meshOut.set_index(currentIndex++, currentVertex-3);
            if (T == metal::topology::triangle || T == metal::topology::line) meshOut.set_index(currentIndex++, currentVertex-2);
            meshOut.set_index(currentIndex++, currentVertex-1);
            meshOut.set_primitive(currentPrimitive++, primitiveData);
        }
    }
};
struct VertexData
{
    float3 normal;
    float4 position;
};

struct main0_out_2
{
};
struct main0_out_1
{
    float3 vNormal [[user(locn0)]];
    float4 gl_Position [[position]];
};

struct main0_out_2_1
{
};
struct main0_in
{
    spvUnsafeArray<VertexData, 2> vin;
};

enum { VERTEX_COUNT = 2, PRIMITIVE_COUNT = 1 };
using mesh_stream_t = spvMeshStream<main0_out_1, main0_out_2_1, VERTEX_COUNT, PRIMITIVE_COUNT, metal::topology::line>;
void main0(mesh_stream_t::mesh_t spvMeshOut, main0_in in)
{
    main0_out_1 out = {};
    main0_out_2_1 out_1 = {};
    mesh_stream_t meshStream(spvMeshOut, out, out_1);
    out.gl_Position = in.vin[0].position;
    out.vNormal = in.vin[0].normal;
    meshStream.EmitVertex();
    out.gl_Position = in.vin[1].position;
    out.vNormal = in.vin[1].normal;
    meshStream.EmitVertex();
    meshStream.EndPrimitive();
}

struct Payload
{
    struct
    {
        struct 
        {
            VertexData vin [[user(locn0)]];
        } in;
    } vertices[2];
};
[[mesh]] void main0(mesh_stream_t::mesh_t outputMesh, const object_data Payload &payload [[payload]],

uint lid [[thread_index_in_threadgroup]], uint tid [[threadgroup_position_in_grid]])
{
    main0_in in;
    const unsigned long vertexCount = 2;
    for (unsigned long i = 0; i < vertexCount; ++i)
    {
        auto out = payload.vertices[i];
        if (i < sizeof(in.vin) / sizeof(in.vin[0]))
        	in.vin[i] = out.in.vin;
    }
    main0(outputMesh, in
    );
}
