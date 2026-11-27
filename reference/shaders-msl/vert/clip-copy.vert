#pragma clang diagnostic ignored "-Wmissing-prototypes"
#pragma clang diagnostic ignored "-Wmissing-braces"

#include <metal_stdlib>
#include <simd/simd.h>

using namespace metal;

template<typename T, uint Num>
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

template <typename T> struct spvIsArray { enum : bool { value = false }; };
template <typename T> struct spvIsArray<T[]> { enum : bool { value = true }; };
template <typename T, uint N> struct spvIsArray<T[N]> { enum : bool { value = true }; };

template<typename T, uint N>
inline enable_if_t<!spvIsArray<T>::value> spvArrayCopyFromConstantToStack(thread T (&dst)[N], constant T (&src)[N])
{
    for (uint i = 0; i < N; i++)
    {
        dst[i] = src[i];
    }
}

template<typename Tdst, typename Tsrc, uint N>
inline enable_if_t<!spvIsArray<Tdst>::value> spvArrayCopyFromConstantToStack(thread Tdst (&dst)[N], constant spvUnsafeArray<Tsrc, N> &src)
{
    spvArrayCopyFromConstantToStack(dst, src.elements);
}
template<typename Tdst, typename Tsrc, uint N>
inline enable_if_t<!spvIsArray<Tsrc>::value> spvArrayCopyFromConstantToStack(thread spvUnsafeArray<Tdst, N> &dst, constant Tsrc (&src)[N])
{
    spvArrayCopyFromConstantToStack(dst.elements, src);
}
template<typename Tdst, typename Tsrc, uint N>
inline void spvArrayCopyFromConstantToStack(thread spvUnsafeArray<Tdst, N> &dst, constant spvUnsafeArray<Tsrc, N> &src)
{
    spvArrayCopyFromConstantToStack(dst.elements, src.elements);
}
template<typename T, uint N>
inline enable_if_t<!spvIsArray<T>::value> spvArrayCopyFromConstantToThreadGroup(threadgroup T (&dst)[N], constant T (&src)[N])
{
    for (uint i = 0; i < N; i++)
    {
        dst[i] = src[i];
    }
}

template<typename Tdst, typename Tsrc, uint N>
inline enable_if_t<!spvIsArray<Tdst>::value> spvArrayCopyFromConstantToThreadGroup(threadgroup Tdst (&dst)[N], constant spvUnsafeArray<Tsrc, N> &src)
{
    spvArrayCopyFromConstantToThreadGroup(dst, src.elements);
}
template<typename Tdst, typename Tsrc, uint N>
inline enable_if_t<!spvIsArray<Tsrc>::value> spvArrayCopyFromConstantToThreadGroup(threadgroup spvUnsafeArray<Tdst, N> &dst, constant Tsrc (&src)[N])
{
    spvArrayCopyFromConstantToThreadGroup(dst.elements, src);
}
template<typename Tdst, typename Tsrc, uint N>
inline void spvArrayCopyFromConstantToThreadGroup(threadgroup spvUnsafeArray<Tdst, N> &dst, constant spvUnsafeArray<Tsrc, N> &src)
{
    spvArrayCopyFromConstantToThreadGroup(dst.elements, src.elements);
}
template<typename T, uint N>
inline enable_if_t<!spvIsArray<T>::value> spvArrayCopyFromStackToStack(thread T (&dst)[N], thread const T (&src)[N])
{
    for (uint i = 0; i < N; i++)
    {
        dst[i] = src[i];
    }
}

template<typename Tdst, typename Tsrc, uint N>
inline enable_if_t<!spvIsArray<Tdst>::value> spvArrayCopyFromStackToStack(thread Tdst (&dst)[N], thread const spvUnsafeArray<Tsrc, N> &src)
{
    spvArrayCopyFromStackToStack(dst, src.elements);
}
template<typename Tdst, typename Tsrc, uint N>
inline enable_if_t<!spvIsArray<Tsrc>::value> spvArrayCopyFromStackToStack(thread spvUnsafeArray<Tdst, N> &dst, thread const Tsrc (&src)[N])
{
    spvArrayCopyFromStackToStack(dst.elements, src);
}
template<typename Tdst, typename Tsrc, uint N>
inline void spvArrayCopyFromStackToStack(thread spvUnsafeArray<Tdst, N> &dst, thread const spvUnsafeArray<Tsrc, N> &src)
{
    spvArrayCopyFromStackToStack(dst.elements, src.elements);
}
template<typename T, uint N>
inline enable_if_t<!spvIsArray<T>::value> spvArrayCopyFromStackToThreadGroup(threadgroup T (&dst)[N], thread const T (&src)[N])
{
    for (uint i = 0; i < N; i++)
    {
        dst[i] = src[i];
    }
}

template<typename Tdst, typename Tsrc, uint N>
inline enable_if_t<!spvIsArray<Tdst>::value> spvArrayCopyFromStackToThreadGroup(threadgroup Tdst (&dst)[N], thread const spvUnsafeArray<Tsrc, N> &src)
{
    spvArrayCopyFromStackToThreadGroup(dst, src.elements);
}
template<typename Tdst, typename Tsrc, uint N>
inline enable_if_t<!spvIsArray<Tsrc>::value> spvArrayCopyFromStackToThreadGroup(threadgroup spvUnsafeArray<Tdst, N> &dst, thread const Tsrc (&src)[N])
{
    spvArrayCopyFromStackToThreadGroup(dst.elements, src);
}
template<typename Tdst, typename Tsrc, uint N>
inline void spvArrayCopyFromStackToThreadGroup(threadgroup spvUnsafeArray<Tdst, N> &dst, thread const spvUnsafeArray<Tsrc, N> &src)
{
    spvArrayCopyFromStackToThreadGroup(dst.elements, src.elements);
}
template<typename T, uint N>
inline enable_if_t<!spvIsArray<T>::value> spvArrayCopyFromThreadGroupToStack(thread T (&dst)[N], threadgroup const T (&src)[N])
{
    for (uint i = 0; i < N; i++)
    {
        dst[i] = src[i];
    }
}

template<typename Tdst, typename Tsrc, uint N>
inline enable_if_t<!spvIsArray<Tdst>::value> spvArrayCopyFromThreadGroupToStack(thread Tdst (&dst)[N], threadgroup const spvUnsafeArray<Tsrc, N> &src)
{
    spvArrayCopyFromThreadGroupToStack(dst, src.elements);
}
template<typename Tdst, typename Tsrc, uint N>
inline enable_if_t<!spvIsArray<Tsrc>::value> spvArrayCopyFromThreadGroupToStack(thread spvUnsafeArray<Tdst, N> &dst, threadgroup const Tsrc (&src)[N])
{
    spvArrayCopyFromThreadGroupToStack(dst.elements, src);
}
template<typename Tdst, typename Tsrc, uint N>
inline void spvArrayCopyFromThreadGroupToStack(thread spvUnsafeArray<Tdst, N> &dst, threadgroup const spvUnsafeArray<Tsrc, N> &src)
{
    spvArrayCopyFromThreadGroupToStack(dst.elements, src.elements);
}
template<typename T, uint N>
inline enable_if_t<!spvIsArray<T>::value> spvArrayCopyFromThreadGroupToThreadGroup(threadgroup T (&dst)[N], threadgroup const T (&src)[N])
{
    for (uint i = 0; i < N; i++)
    {
        dst[i] = src[i];
    }
}

template<typename Tdst, typename Tsrc, uint N>
inline enable_if_t<!spvIsArray<Tdst>::value> spvArrayCopyFromThreadGroupToThreadGroup(threadgroup Tdst (&dst)[N], threadgroup const spvUnsafeArray<Tsrc, N> &src)
{
    spvArrayCopyFromThreadGroupToThreadGroup(dst, src.elements);
}
template<typename Tdst, typename Tsrc, uint N>
inline enable_if_t<!spvIsArray<Tsrc>::value> spvArrayCopyFromThreadGroupToThreadGroup(threadgroup spvUnsafeArray<Tdst, N> &dst, threadgroup const Tsrc (&src)[N])
{
    spvArrayCopyFromThreadGroupToThreadGroup(dst.elements, src);
}
template<typename Tdst, typename Tsrc, uint N>
inline void spvArrayCopyFromThreadGroupToThreadGroup(threadgroup spvUnsafeArray<Tdst, N> &dst, threadgroup const spvUnsafeArray<Tsrc, N> &src)
{
    spvArrayCopyFromThreadGroupToThreadGroup(dst.elements, src.elements);
}
template<typename T, uint N>
inline enable_if_t<!spvIsArray<T>::value> spvArrayCopyFromDeviceToDevice(device T (&dst)[N], device const T (&src)[N])
{
    for (uint i = 0; i < N; i++)
    {
        dst[i] = src[i];
    }
}

template<typename Tdst, typename Tsrc, uint N>
inline enable_if_t<!spvIsArray<Tdst>::value> spvArrayCopyFromDeviceToDevice(device Tdst (&dst)[N], device const spvUnsafeArray<Tsrc, N> &src)
{
    spvArrayCopyFromDeviceToDevice(dst, src.elements);
}
template<typename Tdst, typename Tsrc, uint N>
inline enable_if_t<!spvIsArray<Tsrc>::value> spvArrayCopyFromDeviceToDevice(device spvUnsafeArray<Tdst, N> &dst, device const Tsrc (&src)[N])
{
    spvArrayCopyFromDeviceToDevice(dst.elements, src);
}
template<typename Tdst, typename Tsrc, uint N>
inline void spvArrayCopyFromDeviceToDevice(device spvUnsafeArray<Tdst, N> &dst, device const spvUnsafeArray<Tsrc, N> &src)
{
    spvArrayCopyFromDeviceToDevice(dst.elements, src.elements);
}
template<typename T, uint N>
inline enable_if_t<!spvIsArray<T>::value> spvArrayCopyFromConstantToDevice(device T (&dst)[N], constant T (&src)[N])
{
    for (uint i = 0; i < N; i++)
    {
        dst[i] = src[i];
    }
}

template<typename Tdst, typename Tsrc, uint N>
inline enable_if_t<!spvIsArray<Tdst>::value> spvArrayCopyFromConstantToDevice(device Tdst (&dst)[N], constant spvUnsafeArray<Tsrc, N> &src)
{
    spvArrayCopyFromConstantToDevice(dst, src.elements);
}
template<typename Tdst, typename Tsrc, uint N>
inline enable_if_t<!spvIsArray<Tsrc>::value> spvArrayCopyFromConstantToDevice(device spvUnsafeArray<Tdst, N> &dst, constant Tsrc (&src)[N])
{
    spvArrayCopyFromConstantToDevice(dst.elements, src);
}
template<typename Tdst, typename Tsrc, uint N>
inline void spvArrayCopyFromConstantToDevice(device spvUnsafeArray<Tdst, N> &dst, constant spvUnsafeArray<Tsrc, N> &src)
{
    spvArrayCopyFromConstantToDevice(dst.elements, src.elements);
}
template<typename T, uint N>
inline enable_if_t<!spvIsArray<T>::value> spvArrayCopyFromStackToDevice(device T (&dst)[N], thread const T (&src)[N])
{
    for (uint i = 0; i < N; i++)
    {
        dst[i] = src[i];
    }
}

template<typename Tdst, typename Tsrc, uint N>
inline enable_if_t<!spvIsArray<Tdst>::value> spvArrayCopyFromStackToDevice(device Tdst (&dst)[N], thread const spvUnsafeArray<Tsrc, N> &src)
{
    spvArrayCopyFromStackToDevice(dst, src.elements);
}
template<typename Tdst, typename Tsrc, uint N>
inline enable_if_t<!spvIsArray<Tsrc>::value> spvArrayCopyFromStackToDevice(device spvUnsafeArray<Tdst, N> &dst, thread const Tsrc (&src)[N])
{
    spvArrayCopyFromStackToDevice(dst.elements, src);
}
template<typename Tdst, typename Tsrc, uint N>
inline void spvArrayCopyFromStackToDevice(device spvUnsafeArray<Tdst, N> &dst, thread const spvUnsafeArray<Tsrc, N> &src)
{
    spvArrayCopyFromStackToDevice(dst.elements, src.elements);
}
template<typename T, uint N>
inline enable_if_t<!spvIsArray<T>::value> spvArrayCopyFromThreadGroupToDevice(device T (&dst)[N], threadgroup const T (&src)[N])
{
    for (uint i = 0; i < N; i++)
    {
        dst[i] = src[i];
    }
}

template<typename Tdst, typename Tsrc, uint N>
inline enable_if_t<!spvIsArray<Tdst>::value> spvArrayCopyFromThreadGroupToDevice(device Tdst (&dst)[N], threadgroup const spvUnsafeArray<Tsrc, N> &src)
{
    spvArrayCopyFromThreadGroupToDevice(dst, src.elements);
}
template<typename Tdst, typename Tsrc, uint N>
inline enable_if_t<!spvIsArray<Tsrc>::value> spvArrayCopyFromThreadGroupToDevice(device spvUnsafeArray<Tdst, N> &dst, threadgroup const Tsrc (&src)[N])
{
    spvArrayCopyFromThreadGroupToDevice(dst.elements, src);
}
template<typename Tdst, typename Tsrc, uint N>
inline void spvArrayCopyFromThreadGroupToDevice(device spvUnsafeArray<Tdst, N> &dst, threadgroup const spvUnsafeArray<Tsrc, N> &src)
{
    spvArrayCopyFromThreadGroupToDevice(dst.elements, src.elements);
}
template<typename T, uint N>
inline enable_if_t<!spvIsArray<T>::value> spvArrayCopyFromDeviceToStack(thread T (&dst)[N], device const T (&src)[N])
{
    for (uint i = 0; i < N; i++)
    {
        dst[i] = src[i];
    }
}

template<typename Tdst, typename Tsrc, uint N>
inline enable_if_t<!spvIsArray<Tdst>::value> spvArrayCopyFromDeviceToStack(thread Tdst (&dst)[N], device const spvUnsafeArray<Tsrc, N> &src)
{
    spvArrayCopyFromDeviceToStack(dst, src.elements);
}
template<typename Tdst, typename Tsrc, uint N>
inline enable_if_t<!spvIsArray<Tsrc>::value> spvArrayCopyFromDeviceToStack(thread spvUnsafeArray<Tdst, N> &dst, device const Tsrc (&src)[N])
{
    spvArrayCopyFromDeviceToStack(dst.elements, src);
}
template<typename Tdst, typename Tsrc, uint N>
inline void spvArrayCopyFromDeviceToStack(thread spvUnsafeArray<Tdst, N> &dst, device const spvUnsafeArray<Tsrc, N> &src)
{
    spvArrayCopyFromDeviceToStack(dst.elements, src.elements);
}
template<typename T, uint N>
inline enable_if_t<!spvIsArray<T>::value> spvArrayCopyFromDeviceToThreadGroup(threadgroup T (&dst)[N], device const T (&src)[N])
{
    for (uint i = 0; i < N; i++)
    {
        dst[i] = src[i];
    }
}

template<typename Tdst, typename Tsrc, uint N>
inline enable_if_t<!spvIsArray<Tdst>::value> spvArrayCopyFromDeviceToThreadGroup(threadgroup Tdst (&dst)[N], device const spvUnsafeArray<Tsrc, N> &src)
{
    spvArrayCopyFromDeviceToThreadGroup(dst, src.elements);
}
template<typename Tdst, typename Tsrc, uint N>
inline enable_if_t<!spvIsArray<Tsrc>::value> spvArrayCopyFromDeviceToThreadGroup(threadgroup spvUnsafeArray<Tdst, N> &dst, device const Tsrc (&src)[N])
{
    spvArrayCopyFromDeviceToThreadGroup(dst.elements, src);
}
template<typename Tdst, typename Tsrc, uint N>
inline void spvArrayCopyFromDeviceToThreadGroup(threadgroup spvUnsafeArray<Tdst, N> &dst, device const spvUnsafeArray<Tsrc, N> &src)
{
    spvArrayCopyFromDeviceToThreadGroup(dst.elements, src.elements);
}
struct Block
{
    spvUnsafeArray<float, 4> block0;
};

constant spvUnsafeArray<float, 4> _30 = spvUnsafeArray<float, 4>({ 1.0, 2.0, -1.0, -2.0 });

struct main0_out
{
    float F_array_0 [[user(locn0)]];
    float F_array_1 [[user(locn1)]];
    float F_array_2 [[user(locn2)]];
    float F_array_3 [[user(locn3)]];
    float m_43_block0_0 [[user(locn4)]];
    float m_43_block0_1 [[user(locn5)]];
    float m_43_block0_2 [[user(locn6)]];
    float m_43_block0_3 [[user(locn7)]];
    float4 gl_Position [[position]];
    float gl_ClipDistance [[clip_distance]] [4];
    float gl_ClipDistance_0 [[user(clip0)]];
    float gl_ClipDistance_1 [[user(clip1)]];
    float gl_ClipDistance_2 [[user(clip2)]];
    float gl_ClipDistance_3 [[user(clip3)]];
};

static inline __attribute__((always_inline))
void in_func(thread float4& gl_Position, thread float (&gl_ClipDistance)[4], thread spvUnsafeArray<float, 4>& F_array, thread Block& _43)
{
    gl_Position = float4(1.0, 2.0, 3.0, 4.0);
    spvArrayCopyFromConstantToStack(gl_ClipDistance, _30);
    spvUnsafeArray<float, 4> _36;
    _36[0] = gl_ClipDistance[0];
    _36[1] = gl_ClipDistance[1];
    _36[2] = gl_ClipDistance[2];
    _36[3] = gl_ClipDistance[3];
    spvUnsafeArray<float, 4> non_const_clips = _36;
    spvArrayCopyFromStackToStack(gl_ClipDistance, non_const_clips);
    F_array = non_const_clips;
    _43.block0 = _30;
}

vertex main0_out main0()
{
    main0_out out = {};
    spvUnsafeArray<float, 4> F_array = {};
    Block _43 = {};
    in_func(out.gl_Position, out.gl_ClipDistance, F_array, _43);
    out.gl_ClipDistance_0 = out.gl_ClipDistance[0];
    out.gl_ClipDistance_1 = out.gl_ClipDistance[1];
    out.gl_ClipDistance_2 = out.gl_ClipDistance[2];
    out.gl_ClipDistance_3 = out.gl_ClipDistance[3];
    out.F_array_0 = F_array[0];
    out.F_array_1 = F_array[1];
    out.F_array_2 = F_array[2];
    out.F_array_3 = F_array[3];
    out.m_43_block0_0 = _43.block0[0];
    out.m_43_block0_1 = _43.block0[1];
    out.m_43_block0_2 = _43.block0[2];
    out.m_43_block0_3 = _43.block0[3];
    return out;
}

