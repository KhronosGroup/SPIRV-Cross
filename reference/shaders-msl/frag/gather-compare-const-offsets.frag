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

template<typename T> struct spvRemoveReference { typedef T type; };
template<typename T> struct spvRemoveReference<thread T&> { typedef T type; };
template<typename T> struct spvRemoveReference<thread T&&> { typedef T type; };
template<typename T> inline constexpr thread T&& spvForward(thread typename spvRemoveReference<T>::type& x)
{
    return static_cast<thread T&&>(x);
}
template<typename T> inline constexpr thread T&& spvForward(thread typename spvRemoveReference<T>::type&& x)
{
    return static_cast<thread T&&>(x);
}

// Wrapper function that processes a device texture gather with a constant offset array.
template<typename T, template<typename, access = access::sample, typename = void> class Tex, typename Toff, typename... Tp>
inline vec<T, 4> spvGatherCompareConstOffsets(const device Tex<T>& t, sampler s, Toff coffsets, Tp... params)
{
    vec<T, 4> rslts[4];
    for (uint i = 0; i < 4; i++)
    {
            rslts[i] = t.gather_compare(s, spvForward<Tp>(params)..., coffsets[i]);
    }
    return vec<T, 4>(rslts[0].w, rslts[1].w, rslts[2].w, rslts[3].w);
}

// Wrapper function that processes a constant texture gather with a constant offset array.
template<typename T, template<typename, access = access::sample, typename = void> class Tex, typename Toff, typename... Tp>
inline vec<T, 4> spvGatherCompareConstOffsets(const constant Tex<T>& t, sampler s, Toff coffsets, Tp... params)
{
    vec<T, 4> rslts[4];
    for (uint i = 0; i < 4; i++)
    {
            rslts[i] = t.gather_compare(s, spvForward<Tp>(params)..., coffsets[i]);
    }
    return vec<T, 4>(rslts[0].w, rslts[1].w, rslts[2].w, rslts[3].w);
}

// Wrapper function that processes a thread texture gather with a constant offset array.
template<typename T, template<typename, access = access::sample, typename = void> class Tex, typename Toff, typename... Tp>
inline vec<T, 4> spvGatherCompareConstOffsets(const thread Tex<T>& t, sampler s, Toff coffsets, Tp... params)
{
    vec<T, 4> rslts[4];
    for (uint i = 0; i < 4; i++)
    {
            rslts[i] = t.gather_compare(s, spvForward<Tp>(params)..., coffsets[i]);
    }
    return vec<T, 4>(rslts[0].w, rslts[1].w, rslts[2].w, rslts[3].w);
}

constant spvUnsafeArray<int2, 4> _38 = spvUnsafeArray<int2, 4>({ int2(-8, 3), int2(-4, 7), int2(0, 3), int2(3, 0) });

struct main0_out
{
    float4 FragColor [[color(0)]];
};

struct main0_in
{
    float2 coord [[user(locn0)]];
    float2 compare_value [[user(locn1)]];
};

fragment main0_out main0(main0_in in [[stage_in]], depth2d<float> tex [[texture(0)]], sampler texSmplr [[sampler(0)]])
{
    main0_out out = {};
    out.FragColor = spvGatherCompareConstOffsets(tex, texSmplr, _38, in.coord, in.compare_value.x);
    return out;
}

