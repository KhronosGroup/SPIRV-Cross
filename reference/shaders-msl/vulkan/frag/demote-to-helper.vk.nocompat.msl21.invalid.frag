#include <metal_stdlib>
#include <simd/simd.h>

using namespace metal;

fragment void main0()
{
    bool helper = simd_is_helper_thread();
}

