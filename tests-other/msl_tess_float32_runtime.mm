// Copyright 2026 Jean-Philippe Meunier
// SPDX-License-Identifier: Apache-2.0
#import <Foundation/Foundation.h>
#import <Metal/Metal.h>
#include <cstring>
#include <iostream>
#include <stdexcept>
#include <string>
static void check(bool value, const char *message)
{
    if (!value) throw std::runtime_error(message);
}
static id<MTLComputePipelineState> pipeline(id<MTLDevice> device, const std::string &path)
{
    NSError *error = nil;
    NSString *source = [NSString stringWithContentsOfFile:[NSString stringWithUTF8String:path.c_str()] encoding:NSUTF8StringEncoding error:&error];
    check(source != nil, "Cannot read generated MSL");
    MTLCompileOptions *options = [MTLCompileOptions new];
    options.languageVersion = MTLLanguageVersion2_4;
    id<MTLLibrary> library = [device newLibraryWithSource:source options:options error:&error];
    if (!library) throw std::runtime_error([[error description] UTF8String]);
    id<MTLComputePipelineState> state = [device newComputePipelineStateWithFunction:[library newFunctionWithName:@"main0"] error:&error];
    if (!state) throw std::runtime_error([[error description] UTF8String]);
    return state;
}
static uint32_t bits(float value)
{
    uint32_t result;
    std::memcpy(&result, &value, 4);
    return result;
}
static void equal(float actual, float expected)
{
    if (bits(actual) != bits(expected))
    {
        std::cerr << "actual bits=" << std::hex << bits(actual) << " expected=" << bits(expected) << std::dec << '\n';
        throw std::runtime_error("GPU factor precision/ABI mismatch");
    }
}
int main(int argc, char **argv)
{
    @autoreleasepool
    {
        try
        {
            check(argc == 2, "Usage: runtime evidence-directory");
            std::string directory = argv[1];
            id<MTLDevice> device = MTLCreateSystemDefaultDevice();
            check(device != nil, "No Metal device; GPU proof unavailable");
            std::cout << "Device: " << [[device name] UTF8String] << '\n';
            id<MTLCommandQueue> queue = [device newCommandQueue];
            const uint32_t params[] = {2, 2, 0, 0};
            uint32_t invocations[20] = {2, 0, 0, 0, bits(.25f), bits(.25f), bits(.5f), 0, 0, 0, 0, 0, bits(.25f), bits(.25f), bits(.5f), 1, 1, 0, 0, 0};
            auto buffer = [&](size_t bytes) { return [device newBufferWithLength:bytes options:MTLResourceStorageModeShared]; };
            id<MTLBuffer> factors = buffer(48 + 32), recorded = buffer(96 + 32), observed = buffer(48 + 32), capture = buffer(64 + 32);
            id<MTLBuffer> indirect = [device newBufferWithBytes:params length:sizeof(params) options:MTLResourceStorageModeShared];
            id<MTLBuffer> invocation = [device newBufferWithBytes:invocations length:sizeof(invocations) options:MTLResourceStorageModeShared];
            auto guards = [&](id<MTLBuffer> target, size_t begin) {
                for (size_t offset = begin; offset < target.length; ++offset) check(static_cast<unsigned char *>(target.contents)[offset] == 0xa5, "Buffer guard overwritten");
            };
            for (int mode = 0; mode < 4; ++mode)
            {
                auto tcs = pipeline(device, directory + "/tcs" + std::to_string(mode) + ".metal");
                auto tes = pipeline(device, directory + "/tes" + std::to_string(2 + (mode & 1)) + ".metal");
                for (id<MTLBuffer> target : {factors, recorded, observed, capture}) std::memset(target.contents, 0xa5, target.length);
                auto command = [queue commandBuffer];
                auto encoder = [command computeCommandEncoder];
                [encoder setComputePipelineState:tcs];
                [encoder setBuffer:recorded offset:0 atIndex:0];
                [encoder setBuffer:factors offset:0 atIndex:26];
                [encoder setBuffer:indirect offset:0 atIndex:29];
                [encoder dispatchThreadgroups:MTLSizeMake(mode & 2 ? 1 : 2, 1, 1) threadsPerThreadgroup:MTLSizeMake(mode & 2 ? 4 : 2, 1, 1)];
                [encoder memoryBarrierWithScope:MTLBarrierScopeBuffers];
                [encoder setComputePipelineState:tes];
                [encoder setBuffer:observed offset:0 atIndex:0];
                [encoder setBuffer:factors offset:0 atIndex:26];
                [encoder setBuffer:capture offset:0 atIndex:28];
                [encoder setBuffer:invocation offset:0 atIndex:29];
                [encoder dispatchThreadgroups:MTLSizeMake(1, 1, 1) threadsPerThreadgroup:MTLSizeMake(4, 1, 1)];
                [encoder endEncoding];
                [command commit];
                [command waitUntilCompleted];
                if (command.status != MTLCommandBufferStatusCompleted) throw std::runtime_error([[command.error description] UTF8String]);
                for (int patch = 0; patch < 2; ++patch)
                for (int i = 0; i < 6; ++i)
                {
                    float before = float(patch * 16 + i + 1 + (i >= 2 ? 8 : 0)) + 0x1p-12f;
                    equal(static_cast<float *>(recorded.contents)[patch * 12 + i], before);
                    equal(static_cast<float *>(recorded.contents)[patch * 12 + 6 + i], before + .25f);
                    equal(static_cast<float *>(factors.contents)[patch * 6 + i], before + .25f);
                    equal(static_cast<float *>(observed.contents)[patch * 6 + i], before + .25f);
                }
                for (int patch = 0; patch < 2; ++patch)
                for (int field = 0; field < 2; ++field)
                {
                    auto values = static_cast<float *>(capture.contents) + patch * 8 + field * 4;
                    equal(values[0], static_cast<float *>(factors.contents)[patch * 6 + patch]);
                    equal(values[1], static_cast<float *>(factors.contents)[patch * 6 + 4 + patch]);
                    equal(values[2], .25f);
                    equal(values[3], 1.f);
                }
                guards(factors, 48); guards(recorded, 96); guards(observed, 48); guards(capture, 64);
                std::cout << "PASS TCS -> raw TES mode=" << mode << ": 64 bit-exact values, two 24-byte patches, dynamic tail indices, barriers, guards.\n";
            }
            for (int native = 0; native < 2; ++native)
            {
                auto copy = pipeline(device, directory + "/copy" + std::to_string(native) + ".metal");
                auto tesCopy = pipeline(device, directory + "/tes-copy" + std::to_string(2 + native) + ".metal");
                std::memset(capture.contents, 0xa5, capture.length);
                std::memset(factors.contents, 0xa5, factors.length);
                auto command = [queue commandBuffer];
                auto encoder = [command computeCommandEncoder];
                [encoder setComputePipelineState:copy];
                [encoder setBuffer:factors offset:0 atIndex:26];
                [encoder setBuffer:indirect offset:0 atIndex:29];
                [encoder dispatchThreadgroups:MTLSizeMake(2, 1, 1) threadsPerThreadgroup:MTLSizeMake(1, 1, 1)];
                [encoder memoryBarrierWithScope:MTLBarrierScopeBuffers];
                [encoder setComputePipelineState:tesCopy];
                [encoder setBuffer:capture offset:0 atIndex:28];
                [encoder setBuffer:invocation offset:0 atIndex:29];
                [encoder dispatchThreadgroups:MTLSizeMake(1, 1, 1) threadsPerThreadgroup:MTLSizeMake(4, 1, 1)];
                [encoder endEncoding]; [command commit]; [command waitUntilCompleted];
                if (command.status != MTLCommandBufferStatusCompleted) throw std::runtime_error([[command.error description] UTF8String]);
                for (int patch = 0; patch < 2; ++patch) for (int i = 0; i < 6; ++i) equal(static_cast<float *>(factors.contents)[patch * 6 + i], i == 1 ? 1.f + 0x1p-23f : float(i + 1) + 0x1p-12f);
                const float expected[] = {4.f + 0x1p-12f, 6.f + 0x1p-12f, 1.f + 0x1p-23f, 5.f + 0x1p-12f};
                for (int patch = 0; patch < 2; ++patch) for (int i = 0; i < 4; ++i) equal(static_cast<float *>(capture.contents)[patch * 4 + i], expected[i]);
                guards(factors, 48); guards(capture, 32);
                std::cout << "PASS initializer/OpCopyMemory/OpCopyObject TCS -> TES native=" << native << ": 20 exact values including 1 + 2^-12 and nextafter(1,+inf).\n";
            }
            // Same SPIR-V through the pre-change half compiler must demonstrably lose the tested bit.
            auto half = pipeline(device, directory + "/before-half.metal");
            std::memset(factors.contents, 0xa5, factors.length);
            auto command = [queue commandBuffer]; auto encoder = [command computeCommandEncoder];
            [encoder setComputePipelineState:half]; [encoder setBuffer:factors offset:0 atIndex:26]; [encoder setBuffer:indirect offset:0 atIndex:29];
            [encoder dispatchThreadgroups:MTLSizeMake(2, 1, 1) threadsPerThreadgroup:MTLSizeMake(1, 1, 1)];
            [encoder endEncoding]; [command commit]; [command waitUntilCompleted];
            check(command.status == MTLCommandBufferStatusCompleted, "Half control failed");
            check(static_cast<uint16_t *>(factors.contents)[0] == 0x3c00, "Half precision control did not round to 1.0");
            guards(factors, 16);
            std::cout << "PASS negative precision control: same SPIR-V before change stores half 0x3c00 (1.0), float32 preserves 0x3f800800.\n";
        }
        catch (const std::exception &error) { std::cerr << error.what() << '\n'; return 1; }
    }
}
