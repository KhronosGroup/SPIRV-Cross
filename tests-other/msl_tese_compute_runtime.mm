// Copyright 2026 Jean-Philippe Meunier
// SPDX-License-Identifier: Apache-2.0
// Executes generated TES MSL using a GPU-produced invocation stream.
#import <Foundation/Foundation.h>
#import <Metal/Metal.h>
#include <algorithm>
#include <array>
#include <cmath>
#include <cstring>
#include <fstream>
#include <iostream>
#include <stdexcept>
#include <utility>
#include <vector>
static void check(bool value, const char *message)
{
    if (!value) throw std::runtime_error(message);
}
int main(int argc, char **argv)
{
    @autoreleasepool
    {
        try
        {
            check(argc == 4 || (argc == 5 && std::strcmp(argv[4], "3") == 0), "Usage: runtime evidence-directory fixture topology.metal [3]");
            std::string fixture = argv[2];
            std::string directory = argv[1];
            id<MTLDevice> device = MTLCreateSystemDefaultDevice();
            check(device != nil, "No Metal device; runtime proof unavailable");
            NSError *error = nil;
            NSString *source = [NSString stringWithContentsOfFile:[NSString stringWithUTF8String:(directory + "/" + fixture + ".metal").c_str()] encoding:NSUTF8StringEncoding error:&error];
            check(source != nil, "Cannot read generated MSL");
            MTLCompileOptions *options = [MTLCompileOptions new];
            options.languageVersion = MTLLanguageVersion2_4;
            id<MTLLibrary> library = [device newLibraryWithSource:source options:options error:&error];
            if (!library) throw std::runtime_error([[error description] UTF8String]);
            id<MTLComputePipelineState> pipeline = [device newComputePipelineStateWithFunction:[library newFunctionWithName:@"main0"] error:&error];
            if (!pipeline) throw std::runtime_error([[error description] UTF8String]);
            NSString *replaySource = [NSString stringWithContentsOfFile:[NSString stringWithUTF8String:(directory + "/tese-replay.metal").c_str()] encoding:NSUTF8StringEncoding error:&error];
            check(replaySource != nil, "Cannot read generated TES replay MSL");
            id<MTLLibrary> replayLibrary = [device newLibraryWithSource:replaySource options:options error:&error];
            if (!replayLibrary) throw std::runtime_error([[error description] UTF8String]);
            NSString *fragmentSource = [NSString stringWithContentsOfFile:[NSString stringWithUTF8String:(directory + "/tese-pervertex.metal").c_str()] encoding:NSUTF8StringEncoding error:&error];
            check(fragmentSource != nil, "Cannot read generated PerVertex fragment MSL");
            // Keep the generated PerVertex loads; parameterize only the factor-1 predicate.
            // Fail closed if the generated fixture changes instead of silently testing a different shader.
            NSMutableString *pixelFragmentSource = [fragmentSource mutableCopy];
            const NSArray<NSString *> *needles = @[@"const device uint* spvPerVertexIndices [[buffer(1)]])", @"domain[0] - float3(1.0, 0.0, 0.0)", @"domain[1] - float3(0.0, 1.0, 0.0)", @"domain[2] - float3(0.0, 0.0, 1.0)"];
            const NSArray<NSString *> *replacements = @[@"const device uint* spvPerVertexIndices [[buffer(1)]], constant float4* expectedDomain [[buffer(2)]])", @"domain[0] - expectedDomain[0].xyz", @"domain[1] - expectedDomain[1].xyz", @"domain[2] - expectedDomain[2].xyz"];
            for (NSUInteger i = 0; i < needles.count; ++i)
                check([pixelFragmentSource replaceOccurrencesOfString:needles[i] withString:replacements[i] options:0 range:NSMakeRange(0, pixelFragmentSource.length)] == 1, "Generated PerVertex predicate changed");
            id<MTLLibrary> fragmentLibrary = [device newLibraryWithSource:pixelFragmentSource options:options error:&error];
            if (!fragmentLibrary) throw std::runtime_error([[error description] UTF8String]);
            MTLRenderPipelineDescriptor *replayDescriptor = [MTLRenderPipelineDescriptor new];
            replayDescriptor.vertexFunction = [replayLibrary newFunctionWithName:@"main0"];
            replayDescriptor.fragmentFunction = [fragmentLibrary newFunctionWithName:@"main0"];
            replayDescriptor.colorAttachments[0].pixelFormat = MTLPixelFormatRGBA8Unorm;
            id<MTLRenderPipelineState> replayPipeline = [device newRenderPipelineStateWithDescriptor:replayDescriptor error:&error];
            if (!replayPipeline) throw std::runtime_error([[error description] UTF8String]);
            NSString *topologySource = [NSString stringWithContentsOfFile:[NSString stringWithUTF8String:argv[3]] encoding:NSUTF8StringEncoding error:&error];
            check(topologySource != nil, "Cannot read GPU topology producer");
            id<MTLLibrary> topologyLibrary = [device newLibraryWithSource:topologySource options:options error:&error];
            if (!topologyLibrary) throw std::runtime_error([[error description] UTF8String]);
            id<MTLComputePipelineState> topologyPipeline = [device newComputePipelineStateWithFunction:[topologyLibrary newFunctionWithName:@"generate_tese_invocations"] error:&error];
            if (!topologyPipeline) throw std::runtime_error([[error description] UTF8String]);
            // CPU coordinates below are an oracle only; the TES invocation buffer comes from the GPU.
            const uint32_t factor0 = argc == 5 ? 3u : 2u;
            const std::array<float, 3> a = {1,0,0}, b = {0,1,0}, c = {0,0,1};
            const std::array<float, 3> ab = {.5f,.5f,0}, bc = {0,.5f,.5f}, ca = {.5f,0,.5f}, centre = {1.f/3,1.f/3,1.f/3};
            std::vector<std::array<float, 3>> domains = {a,b,c};
            if (factor0 == 2u) {
                domains.insert(domains.end(), {centre,a,ab, centre,ab,b, centre,b,bc, centre,bc,c, centre,c,ca, centre,ca,a});
            } else {
                const float t = 2.f / 9.f;
                const std::array<float, 3> points[12] = {
                    a, {2.f/3,1.f/3,0}, {1.f/3,2.f/3,0}, b,
                    {0,2.f/3,1.f/3}, {0,1.f/3,2.f/3}, c,
                    {1.f/3,0,2.f/3}, {2.f/3,0,1.f/3},
                    {1.f-2.f*t,t,t}, {t,1.f-2.f*t,t}, {t,t,1.f-2.f*t}
                };
                const uint32_t triangles[13][3] = {
                    {9,8,0}, {9,0,1}, {10,2,3}, {10,3,4}, {11,5,6}, {11,6,7}, {9,10,11},
                    {9,1,10}, {10,1,2}, {10,4,11}, {11,4,5}, {11,7,9}, {9,7,8}
                };
                for (const auto& triangle : triangles) for (uint32_t corner : triangle) domains.push_back(points[corner]);
            }
            const uint32_t count = uint32_t(domains.size());
            std::vector<uint32_t> invocations(4 + count * 8, 0);
            uint32_t settings[3] = {factor0, 1, 1};
            float controls[6][4] = {{2,3,5,1}, {7,11,13,1}, {17,19,23,1}, {29,31,37,1}, {41,43,47,1}, {53,59,61,1}};
            float patches[2][4] = {{1,2,3,0}, {4,5,6,0}};
            const uint16_t factorBits = factor0 == 3u ? 0x4200 : 0x4000;
            uint16_t factors[2][4] = {{factorBits,factorBits,factorBits,factorBits}, {0x3c00,0x3c00,0x3c00,0x3c00}};
            std::ifstream layout(directory + "/" + fixture + ".layout");
            uint32_t stride;
            check(bool(layout >> stride), "Missing exported layout");
            std::vector<std::array<uint32_t, 3>> fields;
            uint32_t location, component, offset;
            while (layout >> location >> component >> offset) fields.push_back({location, component, offset});
            check(fields.size() == 15 && stride >= 60, "Incomplete exported layout");
            auto buffer = [&](const void *bytes, size_t size) {
                id<MTLBuffer> result = [device newBufferWithBytes:bytes length:size options:MTLResourceStorageModeShared];
                check(result != nil, "Cannot allocate test buffer");
                return result;
            };
            id<MTLBuffer> invocationBuffer = buffer(invocations.data(), invocations.size() * 4);
            id<MTLBuffer> settingsBuffer = buffer(settings, sizeof(settings));
            id<MTLBuffer> occurrenceBuffer = [device newBufferWithLength:count * 2 * sizeof(uint32_t) options:MTLResourceStorageModeShared];
            id<MTLBuffer> drawBuffer = [device newBufferWithLength:4 * sizeof(uint32_t) options:MTLResourceStorageModeShared];
            id<MTLBuffer> primitiveIndexBuffer = [device newBufferWithLength:count * sizeof(uint32_t) options:MTLResourceStorageModeShared];
            check(occurrenceBuffer != nil && drawBuffer != nil && primitiveIndexBuffer != nil, "Cannot allocate GPU replay tables");
            id<MTLBuffer> controlBuffer = buffer(controls, sizeof(controls));
            id<MTLBuffer> patchBuffer = buffer(patches, sizeof(patches));
            id<MTLBuffer> factorBuffer = buffer(factors, sizeof(factors));
            id<MTLBuffer> output = [device newBufferWithLength:(count + 1) * stride options:MTLResourceStorageModeShared];
            check(output != nil, "Cannot allocate output");
            id<MTLCommandQueue> queue = [device newCommandQueue];
            auto run = [&](bool produce) {
                id<MTLCommandBuffer> command = [queue commandBuffer];
                if (produce)
                {
                    id<MTLComputeCommandEncoder> producer = [command computeCommandEncoder];
                    [producer setComputePipelineState:topologyPipeline];
                    [producer setBuffer:settingsBuffer offset:0 atIndex:0];
                    [producer setBuffer:invocationBuffer offset:0 atIndex:1];
                    [producer setBuffer:occurrenceBuffer offset:0 atIndex:2];
                    [producer setBuffer:drawBuffer offset:0 atIndex:3];
                    [producer setBuffer:primitiveIndexBuffer offset:0 atIndex:4];
                    [producer dispatchThreads:MTLSizeMake(1,1,1) threadsPerThreadgroup:MTLSizeMake(1,1,1)];
                    [producer endEncoding];
                }
                id<MTLComputeCommandEncoder> encoder = [command computeCommandEncoder];
                [encoder setComputePipelineState:pipeline];
                [encoder setBuffer:invocationBuffer offset:0 atIndex:29];
                [encoder setBuffer:output offset:0 atIndex:28];
                [encoder setBuffer:controlBuffer offset:0 atIndex:22];
                [encoder setBuffer:patchBuffer offset:0 atIndex:20];
                [encoder setBuffer:factorBuffer offset:0 atIndex:26];
                [encoder dispatchThreads:MTLSizeMake(count + 11,2,2) threadsPerThreadgroup:MTLSizeMake(8,1,1)];
                [encoder endEncoding];
                [command commit];
                [command waitUntilCompleted];
                if (command.status != MTLCommandBufferStatusCompleted) throw std::runtime_error([[command.error description] UTF8String]);
            };
            for (uint32_t multiplier : {1u, factor0 == 3u ? 5u : 2u})
            {
                static_cast<uint32_t *>(settingsBuffer.contents)[2] = multiplier;
                std::memset(output.contents, 0xcd, output.length);
                run(true);
                const auto *records = static_cast<const uint32_t *>(invocationBuffer.contents);
                check(records[0] == count && records[1] == 0 && records[2] == 0 && records[3] == 0, "GPU invocation header mismatch");
                const auto *occurrences = static_cast<const uint32_t *>(occurrenceBuffer.contents);
                const auto *primitiveIndices = static_cast<const uint32_t *>(primitiveIndexBuffer.contents);
                const auto *draw = static_cast<const uint32_t *>(drawBuffer.contents);
                check(draw[0] == 0 && draw[1] == 0 && draw[2] == 0 && draw[3] == count, "GPU replay draw parameters mismatch");
                for (uint32_t i = 0; i < count; ++i)
                {
                    const uint32_t *record = records + 4 + 8 * i;
                    float xyz[3];
                    std::memcpy(xyz, record, sizeof(xyz));
                    for (uint32_t component = 0; component < 3; ++component) check(std::fabs(xyz[component] - domains[i][component]) < 0.00001f, "GPU domain coordinate mismatch");
                    check(record[3] == (i < 3 ? 1u : 0u) && record[4] == (i * multiplier) % count, "GPU patch or record index mismatch");
                    check(record[5] == 0 && record[6] == 0 && record[7] == 0, "GPU reserved invocation words differ");
                    check(occurrences[2 * i] == record[4] && occurrences[2 * i + 1] == i / 3, "GPU replay occurrence table mismatch");
                    check(primitiveIndices[i] == record[4], "GPU fragment primitive index table mismatch");
                }
                auto *bytes = static_cast<unsigned char *>(output.contents);
                for (uint32_t i = 0; i < count; ++i)
                {
                    uint32_t patch = i < 3 ? 1 : 0;
                    uint32_t record = (i * multiplier) % count;
                    for (auto field : fields)
                    {
                        uint32_t loc = field[0], c = field[1];
                        if (loc == 1)
                        {
                            int32_t actual;
                            std::memcpy(&actual, bytes + record * stride + field[2], 4);
                            check(actual == int32_t(patch), "Patch ID mismatch");
                            continue;
                        }
                        float expected = 0, actual;
                        if (loc == 0 || loc == 4)
                        {
                            expected = patches[patch][c];
                            for (uint32_t vertex = 0; vertex < 3; ++vertex) expected += controls[patch * 3 + vertex][c] * domains[i][vertex];
                        }
                        else if (loc == 2) expected = domains[i][c];
                        else if (loc == 3) expected = c == 2 ? 3.f : (patch ? 1.f : float(factor0));
                        else throw std::runtime_error("Unexpected field");
                        std::memcpy(&actual, bytes + record * stride + field[2], 4);
                        check(std::fabs(actual - expected) < 0.00001f, "Captured scalar differs from expected TES evaluation");
                    }
                }
                for (uint32_t i = count * stride; i < output.length; ++i) check(bytes[i] == 0xcd, "Dispatch overrun wrote output guard");
            }
            auto *bytes = static_cast<unsigned char *>(output.contents);
            std::memset(output.contents, 0xcd, output.length);
            static_cast<uint32_t *>(invocationBuffer.contents)[0] = 0;
            run(false);
            for (uint32_t i = 0; i < output.length; ++i) check(bytes[i] == 0xcd, "Zero invocation count wrote output");
            const float clip[3][4] = {{-.8f,-.8f,0,1}, {.8f,-.8f,0,1}, {0,.8f,0,1}};
            auto *clipControls = static_cast<float (*)[4]>(controlBuffer.contents);
            for (uint32_t patch = 0; patch < 2; ++patch)
                for (uint32_t vertex = 0; vertex < 3; ++vertex)
                    std::memcpy(clipControls[3 * patch + vertex], clip[vertex], sizeof(clip[vertex]));
            std::memset(patchBuffer.contents, 0, patchBuffer.length);
            MTLTextureDescriptor *textureDescriptor = [MTLTextureDescriptor texture2DDescriptorWithPixelFormat:MTLPixelFormatRGBA8Unorm width:64 height:64 mipmapped:NO];
            textureDescriptor.usage = MTLTextureUsageRenderTarget;
            textureDescriptor.storageMode = MTLStorageModeShared;
            id<MTLTexture> texture = [device newTextureWithDescriptor:textureDescriptor];
            check(texture != nil, "Cannot allocate replay target");
            MTLRenderPassDescriptor *renderPass = [MTLRenderPassDescriptor renderPassDescriptor];
            renderPass.colorAttachments[0].texture = texture;
            renderPass.colorAttachments[0].loadAction = MTLLoadActionClear;
            renderPass.colorAttachments[0].storeAction = MTLStoreActionStore;
            renderPass.colorAttachments[0].clearColor = MTLClearColorMake(0, 0, 0, 1);
            // domains uses the connectivity/coordinates independently checked against native Vulkan
            // images in expanded-reproducer/tessellation/oracle-factor{2,3}-evidence.
            // This establishes our ABI corner order, not Vulkan's ordered PerVertex semantics.
            float expectedDomain[3][4] = {};
            id<MTLBuffer> expectedBuffer = buffer(expectedDomain, sizeof(expectedDomain));
            uint32_t rasterChecks = 0;
            for (uint32_t multiplier : {1u, factor0 == 3u ? 5u : 2u})
            {
                static_cast<uint32_t *>(settingsBuffer.contents)[2] = multiplier;
                std::memset(output.contents, 0xcd, output.length);
                run(true);
                auto *fragmentIndices = static_cast<uint32_t *>(primitiveIndexBuffer.contents);
                for (uint32_t first = 0; first < count; first += 3)
                {
                    std::array<std::array<double, 2>, 3> positions{};
                    for (uint32_t corner = 0; corner < 3; ++corner)
                    {
                        for (uint32_t component = 0; component < 3; ++component) expectedDomain[corner][component] = domains[first + corner][component];
                        for (uint32_t axis = 0; axis < 2; ++axis)
                            for (uint32_t vertex = 0; vertex < 3; ++vertex) positions[corner][axis] += domains[first + corner][vertex] * clip[vertex][axis];
                    }
                    std::memcpy(expectedBuffer.contents, expectedDomain, sizeof(expectedDomain));
                    const auto &a = positions[0], &b = positions[1], &c = positions[2];
                    const double determinant = (b[1] - c[1]) * (a[0] - c[0]) + (c[0] - b[0]) * (a[1] - c[1]);
                    check(std::fabs(determinant) > 0.00001, "Degenerate oracle primitive");
                    auto render = [&](const char *control, bool expectedRed) {
                        id<MTLCommandBuffer> replayCommand = [queue commandBuffer];
                        id<MTLRenderCommandEncoder> renderer = [replayCommand renderCommandEncoderWithDescriptor:renderPass];
                        [renderer setRenderPipelineState:replayPipeline];
                        [renderer setVertexBuffer:output offset:0 atIndex:0];
                        [renderer setVertexBuffer:occurrenceBuffer offset:0 atIndex:1];
                        [renderer setVertexBuffer:drawBuffer offset:0 atIndex:2];
                        [renderer setFragmentBuffer:output offset:0 atIndex:0];
                        [renderer setFragmentBuffer:primitiveIndexBuffer offset:0 atIndex:1];
                        [renderer setFragmentBuffer:expectedBuffer offset:0 atIndex:2];
                        [renderer drawPrimitives:MTLPrimitiveTypeTriangle vertexStart:first vertexCount:3];
                        [renderer endEncoding];
                        [replayCommand commit];
                        [replayCommand waitUntilCompleted];
                        if (replayCommand.status != MTLCommandBufferStatusCompleted) throw std::runtime_error([[replayCommand.error description] UTF8String]);
                        std::vector<uint8_t> pixels(64 * 64 * 4);
                        [texture getBytes:pixels.data() bytesPerRow:64 * 4 fromRegion:MTLRegionMake2D(0, 0, 64, 64) mipmapLevel:0];
                        uint32_t interior = 0, exterior = 0;
                        for (uint32_t y = 0; y < 64; ++y)
                            for (uint32_t x = 0; x < 64; ++x)
                            {
                                const double px = 2.0 * (x + .5) / 64 - 1.0, py = 1.0 - 2.0 * (y + .5) / 64;
                                const double l0 = ((b[1] - c[1]) * (px - c[0]) + (c[0] - b[0]) * (py - c[1])) / determinant;
                                const double l1 = ((c[1] - a[1]) * (px - c[0]) + (a[0] - c[0]) * (py - c[1])) / determinant;
                                const double minimum = std::min({l0, l1, 1.0 - l0 - l1});
                                if (std::fabs(minimum) < .02) continue; // Exclude raster edge rounding/tie rules.
                                const bool inside = minimum > 0;
                                const auto *pixel = pixels.data() + (y * 64 + x) * 4;
                                if (pixel[0] != (inside && expectedRed ? 255 : 0) || pixel[1] != (inside && !expectedRed ? 255 : 0) || pixel[2] != 0 || pixel[3] != 255)
                                    throw std::runtime_error("Pixel mismatch: factor=" + std::to_string(factor0) + " multiplier=" + std::to_string(multiplier) + " primitive=" + std::to_string(first / 3) + " control=" + control + " xy=" + std::to_string(x) + "," + std::to_string(y));
                                inside ? ++interior : ++exterior;
                            }
                        check(interior >= 8 && exterior > 0, "Insufficient oracle pixel coverage");
                        ++rasterChecks;
                        std::cout << "PIXEL PASS factor=" << (first ? factor0 : 1) << " patch=" << (first ? 0 : 1) << " primitive=" << first / 3 << " multiplier=" << multiplier << " control=" << control << " interior=" << interior << " exterior=" << exterior << " rgba=" << (expectedRed ? "255,0,0,255" : "0,255,0,255") << '\n';
                    };
                    render("ordered", true);
                    std::swap(fragmentIndices[first], fragmentIndices[first + 1]);
                    render("swap01", false);
                    std::swap(fragmentIndices[first], fragmentIndices[first + 1]);
                    std::swap(fragmentIndices[first + 1], fragmentIndices[first + 2]);
                    render("swap12", false);
                    std::swap(fragmentIndices[first + 1], fragmentIndices[first + 2]);
                    if (first != 0)
                    {
                        const std::array<uint32_t, 3> saved = {fragmentIndices[first], fragmentIndices[first + 1], fragmentIndices[first + 2]};
                        std::memcpy(fragmentIndices + first, fragmentIndices, sizeof(saved));
                        render("wrong-factor1-triplet", false);
                        std::memcpy(fragmentIndices + first, saved.data(), sizeof(saved));
                    }
                }
            }
            std::cout << "PASS: Metal " << [[device name] UTF8String] << ", factors 1/" << factor0 << ", " << count << " GPU-produced corners, 2 patches, ordered and permuted dense records, all " << (2 * count * fields.size()) << " reflected scalar checks, GPU replay and fragment primitive tables, " << rasterChecks << " isolated primitive raster checks (all factor-1 and factor-" << factor0 << " triangles, ordered/permuted records, red ordered and green swap01/swap12/wrong-patch controls), excess dispatch and zero count\n";
        }
        catch (const std::exception &error) { std::cerr << error.what() << '\n'; return 1; }
    }
}
