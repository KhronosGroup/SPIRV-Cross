// Copyright 2026 Jean-Philippe Meunier
// SPDX-License-Identifier: Apache-2.0
#include "spirv_msl.hpp"
#include <fstream>
#include <functional>
#include <iostream>
#include <stdexcept>
using namespace spirv_cross;
static void check(bool value, const char *message)
{
    if (!value) throw std::runtime_error(message);
}
static std::vector<uint32_t> read_spirv(const std::string &path)
{
    std::ifstream file(path, std::ios::binary | std::ios::ate);
    check(bool(file), "Cannot open fixture");
    auto size = file.tellg();
    check(size > 0 && size % 4 == 0, "Invalid fixture size");
    std::vector<uint32_t> words(size_t(size) / 4);
    file.seekg(0);
    file.read(reinterpret_cast<char *>(words.data()), size);
    check(bool(file), "Cannot read fixture");
    return words;
}
static void configure(CompilerMSL &compiler)
{
    auto options = compiler.get_msl_options();
    options.set_msl_version(2, 4);
    options.capture_output_to_buffer = true;
    options.raw_buffer_tese_input = true;
    options.tese_as_compute = true;
    compiler.set_msl_options(options);
    compiler.set_execution_mode(spv::ExecutionModeOutputVertices, 3);
}
static void rejects(const std::function<void()> &action, const char *message)
{
    try { action(); }
    catch (const CompilerError &error)
    {
        check(std::string(error.what()).find(message) != std::string::npos, error.what());
        return;
    }
    throw std::runtime_error(std::string("Expected rejection: ") + message);
}
int main(int argc, char **argv)
{
    try
    {
        check(argc >= 4, "Usage: test fixture-directory vertex.spv pervertex.frag.spv [output-directory]");
        std::string directory = argv[1];
        auto fixture = [&](const char *name) { return read_spirv(directory + "/msl_tese_compute_" + name + ".spv"); };
        auto words = fixture("triangle");
        auto invalid = [&](const std::function<void(CompilerMSL::Options &)> &change, const char *diagnostic) {
            CompilerMSL compiler(words);
            configure(compiler);
            auto options = compiler.get_msl_options();
            change(options);
            compiler.set_msl_options(options);
            rejects([&]() { compiler.compile(); }, diagnostic);
            rejects([&]() { compiler.get_msl_captured_vertex_layout(); }, "successful MSL compilation");
        };
        invalid([](CompilerMSL::Options &o) { o.tese_as_compute = false; }, "hardware TES");
        invalid([](CompilerMSL::Options &o) { o.capture_output_to_buffer = false; }, "requires a TES shader");
        invalid([](CompilerMSL::Options &o) { o.raw_buffer_tese_input = false; }, "requires a TES shader");
        invalid([](CompilerMSL::Options &o) { o.set_msl_version(2, 3); }, "MSL 2.4");
        for (auto flag : {&CompilerMSL::Options::multiview, &CompilerMSL::Options::vertex_for_tessellation, &CompilerMSL::Options::multi_patch_workgroup, &CompilerMSL::Options::tess_domain_origin_lower_left, &CompilerMSL::Options::argument_buffers, &CompilerMSL::Options::dispatch_base, &CompilerMSL::Options::emulate_depth_clip_enable, &CompilerMSL::Options::emulate_reversed_depth_viewport})
            invalid([&](CompilerMSL::Options &o) { o.*flag = true; }, "does not support");
        invalid([](CompilerMSL::Options &o) { o.shader_output_buffer_index = o.indirect_params_buffer_index; }, "distinct");
        invalid([](CompilerMSL::Options &o) { o.indirect_params_buffer_index = 31; }, "[0, 30]");
        for (uint32_t count : {0u, 33u})
        {
            CompilerMSL compiler(words);
            configure(compiler);
            compiler.set_execution_mode(spv::ExecutionModeOutputVertices, count);
            rejects([&]() { compiler.compile(); }, "OutputVertices override");
        }
        for (const char *name : {"quads", "fractional", "empty"})
        {
            CompilerMSL compiler(fixture(name));
            configure(compiler);
            rejects([&]() { compiler.compile(); }, std::string(name) == "empty" ? "captured output record" : "triangle domain and equal spacing");
        }
        CompilerMSL vertex(read_spirv(argv[2]));
        configure(vertex);
        rejects([&]() { vertex.compile(); }, "requires a TES shader");
        CompilerMSL masked(words);
        configure(masked);
        masked.mask_stage_output_by_location(0, 0);
        rejects([&]() { masked.compile(); }, "masked outputs");
        for (uint32_t binding : {20u, 22u, 26u, 28u, 29u})
        {
            CompilerMSL compiler(fixture("resource"));
            configure(compiler);
            MSLResourceBinding resource;
            resource.stage = spv::ExecutionModelTessellationEvaluation;
            resource.desc_set = 0;
            resource.binding = 0;
            resource.msl_buffer = binding;
            compiler.add_msl_resource_binding(resource);
            rejects([&]() { compiler.compile(); }, "collides");
        }
        for (const char *name : {"triangle", "constant", "resource", "block"})
        {
            CompilerMSL compiler(fixture(name));
            configure(compiler);
            rejects([&]() { compiler.get_msl_captured_vertex_layout(); }, "successful MSL compilation");
            auto source = compiler.compile();
            auto layout = compiler.get_msl_captured_vertex_layout();
            check(source.find("kernel void main0(") != std::string::npos, "Missing kernel entry");
            for (const char *forbidden : {"[[ patch(", "[[patch_id]]", "[[position_in_patch]]", "spvIndirectParams", "_0", "[[stage_in]]"})
                check(source.find(forbidden) == std::string::npos, "Hardware TES or vertex capture leaked into kernel");
            check(source.find("spvTessRecord[4]") != std::string::npos, "Missing explicit output record");
            check(layout.stride % 16 == 0 && layout.builtins.size() == 4, "Invalid position layout");
            source += "\nstatic_assert(sizeof(main0_out) == " + std::to_string(layout.stride) + ", \"capture stride\");\n";
            std::string fields = std::to_string(layout.stride) + "\n";
            const char *members[] = {"value", "patchID", "domain", "factors"};
            for (const auto &field : layout.components)
            {
                check(field.location < 4, "Unexpected location");
                check(field.scalar_type == (field.location == 1 ? SPIRType::Int : SPIRType::Float), "Wrong scalar type");
                source += "static_assert(__builtin_offsetof(main0_out, " + std::string(std::string(name) == "block" ? "result_" : "") + members[field.location] + ") + " + std::to_string(4 * field.component) + " == " + std::to_string(field.byte_offset) + ", \"field offset\");\n";
                fields += std::to_string(field.location) + " " + std::to_string(field.component) + " " + std::to_string(field.byte_offset) + "\n";
            }
            for (const auto &field : layout.builtins)
            {
                check(field.builtin == spv::BuiltInPosition, "Unexpected builtin");
                source += "static_assert(__builtin_offsetof(main0_out, gl_Position) + " + std::to_string(4 * field.component) + " == " + std::to_string(field.byte_offset) + ", \"builtin offset\");\n";
                fields += "4 " + std::to_string(field.component) + " " + std::to_string(field.byte_offset) + "\n";
            }
            if (std::string(name) == "triangle")
            {
                check(layout.components.size() == 11, "Missing user fields");
                CompilerMSL replay(fixture(name));
                auto replayOptions = replay.get_msl_options();
                replayOptions.set_msl_version(2, 4);
                replayOptions.enable_point_size_default = false;
                replay.set_msl_options(replayOptions);
                MSLCapturedOutputReplayBinding replayBinding;
                replayBinding.vertex_buffer_index = 0;
                replayBinding.occurrence_buffer_index = 1;
                replayBinding.draw_parameters_buffer_index = 2;
                replayBinding.primitive_index_location = 5;
                auto replaySource = replay.compile_captured_output_replay(layout, replayBinding);
                check(replaySource.find("vertex main0_out main0(") != std::string::npos, "TES replay did not emit a raster vertex function");
                CompilerMSL fragment(read_spirv(argv[3]));
                auto fragmentOptions = fragment.get_msl_options();
                fragmentOptions.set_msl_version(2, 4);
                fragment.set_msl_options(fragmentOptions);
                MSLPerVertexInputBinding fragmentBinding;
                fragmentBinding.vertex_buffer_index = 0;
                fragmentBinding.primitive_index_buffer_index = 1;
                fragmentBinding.primitive_index_location = replayBinding.primitive_index_location;
                fragment.set_msl_per_vertex_input_buffer(layout, fragmentBinding);
                auto fragmentSource = fragment.compile();
                check(fragment.needs_per_vertex_input_buffer() && fragmentSource.find("spvPerVertexIndices") != std::string::npos, "TES fragment did not consume PerVertex records");
                // A fragment PrimitiveId reads the per-triplet patch table when one is bound, else [[primitive_id]].
                auto primitiveFragment = [&](const std::vector<uint32_t> &spirv, uint32_t table) {
                    CompilerMSL compiler(spirv);
                    compiler.set_msl_options(fragmentOptions);
                    auto binding = fragmentBinding;
                    binding.primitive_id_buffer_index = table;
                    compiler.set_msl_per_vertex_input_buffer(layout, binding);
                    return compiler.compile();
                };
                auto primitiveWords = read_spirv(directory + "/msl_tese_pervertex_primitive.spv");
                auto primitiveSource = primitiveFragment(primitiveWords, 2);
                check(primitiveSource.find("const device uint* spvPerVertexPrimitiveIds [[buffer(2)]]") != std::string::npos &&
                      primitiveSource.find("uint gl_PrimitiveID = spvPerVertexPrimitiveIds[in.spvPerVertexPrimitive];") != std::string::npos &&
                      primitiveSource.find("[[primitive_id]]") == std::string::npos, "Fragment PrimitiveId did not read the patch table");
                auto rasterizedSource = primitiveFragment(primitiveWords, ~0u);
                check(rasterizedSource.find("[[primitive_id]]") != std::string::npos && rasterizedSource.find("spvPerVertexPrimitiveIds") == std::string::npos, "Fragment PrimitiveId without a table changed");
                check(primitiveFragment(read_spirv(argv[3]), 2) == fragmentSource, "An unused PrimitiveId table changed the fragment");
                for (uint32_t table : {fragmentBinding.vertex_buffer_index, fragmentBinding.primitive_index_buffer_index, 31u})
                    rejects([&]() { primitiveFragment(primitiveWords, table); }, "PrimitiveId buffer requires a distinct Metal buffer index");
                if (argc > 4)
                {
                    std::ofstream replayFile(std::string(argv[4]) + "/tese-replay.metal");
                    replayFile << replaySource;
                    check(bool(replayFile), "Cannot write TES replay MSL evidence");
                    std::ofstream fragmentFile(std::string(argv[4]) + "/tese-pervertex.metal");
                    fragmentFile << fragmentSource;
                    check(bool(fragmentFile), "Cannot write TES PerVertex fragment MSL evidence");
                    std::ofstream primitiveFile(std::string(argv[4]) + "/tese-pervertex-primitive.metal");
                    primitiveFile << primitiveSource;
                    check(bool(primitiveFile), "Cannot write TES PerVertex PrimitiveId MSL evidence");
                }
            }
            if (argc > 4)
            {
                std::ofstream metal(std::string(argv[4]) + "/" + name + ".metal");
                metal << source;
                check(bool(metal), "Cannot write MSL evidence");
                std::ofstream metadata(std::string(argv[4]) + "/" + name + ".layout");
                metadata << fields;
                check(bool(metadata), "Cannot write layout evidence");
            }
        }
        std::cout << "PASS: TES compute API, captured layout, constants, resources and rejected configurations\n";
    }
    catch (const std::exception &error) { std::cerr << error.what() << '\n'; return 1; }
}
