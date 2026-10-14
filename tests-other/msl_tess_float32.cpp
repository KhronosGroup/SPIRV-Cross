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
static std::vector<uint32_t> read_spirv(const char *path)
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
static void configure(CompilerMSL &compiler, bool fp32 = true)
{
    auto options = compiler.get_msl_options();
    options.set_msl_version(2, 4);
    options.raw_buffer_tese_input = compiler.get_execution_model() == spv::ExecutionModelTessellationEvaluation;
    options.tessellation_factors_float32 = fp32;
    compiler.set_msl_options(options);
}
static void rejects(CompilerMSL &compiler, const char *message)
{
    try { compiler.compile(); }
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
        check(argc == 8 || argc == 9, "Usage: test tesc.spv tese.spv copy.spv tes-copy.spv block.spv parameter.spv vertex.spv [output-directory]");
        auto tcs = read_spirv(argv[1]), tes = read_spirv(argv[2]), copy = read_spirv(argv[3]);
        check(!CompilerMSL::Options().tessellation_factors_float32, "Float32 must be opt-in");
        for (int fixture = 0; fixture < 4; fixture++)
        for (int mode = 0; mode < 4; mode++)
        {
            CompilerMSL compiler(fixture == 0 ? tcs : fixture == 1 ? tes : fixture == 2 ? copy : read_spirv(argv[4]));
            configure(compiler);
            auto options = compiler.get_msl_options();
            options.force_native_arrays = (mode & 1) != 0;
            options.multi_patch_workgroup = fixture == 0 && (mode & 2) != 0;
            options.tese_as_compute = (fixture == 1 || fixture == 3) && (mode & 2) != 0;
            options.capture_output_to_buffer = options.tese_as_compute;
            compiler.set_msl_options(options);
            compiler.set_execution_mode(spv::ExecutionModeOutputVertices, fixture == 2 ? 1 : 2);
            compiler.set_execution_mode(spv::ExecutionModeTriangles);
            auto source = compiler.compile();
            if (options.tese_as_compute) check(compiler.get_msl_captured_vertex_layout().stride == (fixture == 1 ? 32u : 16u), "Unexpected TES capture stride");
            check(source.find("float outer[4];") != std::string::npos && source.find("float inner[2];") != std::string::npos, "Wrong ABI");
            check(source.find("half") == std::string::npos, "Float32 factors were narrowed");
            check(source.find(".outer[3]") != std::string::npos || source.find("gl_TessLevelOuter[3]") != std::string::npos, "Lost fourth outer factor");
            check(source.find(".inner[1]") != std::string::npos || source.find("gl_TessLevelInner[1]") != std::string::npos, "Lost second inner factor");
            if (fixture == 0) check(source.find("mem_flags::mem_device | mem_flags::mem_threadgroup") != std::string::npos, "Missing factor barrier");
            if (argc == 9)
            {
                std::ofstream file(std::string(argv[8]) + "/" + (fixture == 0 ? "tcs" : fixture == 1 ? "tes" : fixture == 2 ? "copy" : "tes-copy") + std::to_string(mode) + ".metal");
                file << source << "\nstatic_assert(sizeof(spvTessellationFactorsFloat) == 24, \"factor stride\");\nstatic_assert(alignof(spvTessellationFactorsFloat) == 4, \"factor alignment\");\nstatic_assert(__builtin_offsetof(spvTessellationFactorsFloat, inner) == 16, \"inner offset\");\n";
                check(bool(file), "Cannot write MSL");
            }
        }
        for (const auto &words : {tcs, tes, copy})
        {
            CompilerMSL explicitFalse(words), implicitFalse(words);
            configure(explicitFalse, false);
            auto options = implicitFalse.get_msl_options();
            options.set_msl_version(2, 4);
            options.raw_buffer_tese_input = implicitFalse.get_execution_model() == spv::ExecutionModelTessellationEvaluation;
            implicitFalse.set_msl_options(options);
            check(explicitFalse.compile() == implicitFalse.compile(), "Explicit false changed default half output");
        }
        for (int mode = 0; mode < 3; mode++)
        {
            CompilerMSL compiler(mode == 2 ? tcs : tes);
            configure(compiler);
            auto options = compiler.get_msl_options();
            options.emulate_reversed_depth_viewport = mode != 0;
            options.reversed_depth_viewport_buffer_index = mode == 1 ? 18 : options.shader_tess_factor_buffer_index;
            compiler.set_msl_options(options);
            auto source = compiler.compile();
            bool emitted = source.find("constant uint& spvEmulatedReversedDepthViewportMask [[buffer(18)]]") != std::string::npos;
            check(emitted == (mode == 1), "Unexpected reversed-depth buffer emission");
            if (mode != 1) check(source.find("spvEmulatedReversedDepthViewportMask") == std::string::npos, "Inactive reversed-depth buffer was emitted");
            if (argc == 9)
            {
                std::ofstream file(std::string(argv[8]) + "/reversed-" + (mode == 0 ? "disabled" : mode == 1 ? "distinct" : "tcs") + ".metal");
                file << source;
                check(bool(file), "Cannot write reversed-depth MSL");
            }
        }
        for (uint32_t slot : {26u, 18u})
        {
            CompilerMSL compiler(tes);
            configure(compiler);
            auto options = compiler.get_msl_options();
            options.emulate_reversed_depth_viewport = true;
            options.shader_tess_factor_buffer_index = slot;
            options.reversed_depth_viewport_buffer_index = slot;
            compiler.set_msl_options(options);
            try
            {
                auto source = compiler.compile();
                if (argc == 9)
                {
                    std::ofstream file(std::string(argv[8]) + "/unexpected-reversed-collision" + std::to_string(slot) + ".metal");
                    file << source;
                    check(bool(file), "Cannot write unexpected collision MSL");
                }
                throw std::runtime_error("Expected rejection: Float32 TessLevel reversed-depth buffer collision");
            }
            catch (const CompilerError &error)
            {
                check(std::string(error.what()).find("Float32 TessLevel buffer binding collides with an application or auxiliary buffer.") != std::string::npos, error.what());
                std::cout << "PASS: reversed-depth collision at buffer(" << slot << ") rejected: " << error.what() << '\n';
            }
        }
        auto invalid = [&](const std::vector<uint32_t> &words, const std::function<void(CompilerMSL &, CompilerMSL::Options &)> &change, const char *message) {
            CompilerMSL compiler(words);
            configure(compiler);
            auto options = compiler.get_msl_options();
            change(compiler, options);
            compiler.set_msl_options(options);
            rejects(compiler, message);
        };
        invalid(read_spirv(argv[5]), [](CompilerMSL &, CompilerMSL::Options &) {}, "standalone builtins");
        invalid(read_spirv(argv[6]), [](CompilerMSL &, CompilerMSL::Options &) {}, "explicit IO pointer");
        invalid(copy, [](CompilerMSL &c, CompilerMSL::Options &) { c.set_execution_mode(spv::ExecutionModeOutputVertices, 2); }, "initializers currently require OutputVertices 1");
        invalid(copy, [](CompilerMSL &c, CompilerMSL::Options &) {
            for (const auto &builtin : c.get_shader_resources().builtin_outputs)
                if (builtin.builtin == spv::BuiltInTessLevelOuter) c.unset_decoration(builtin.resource.id, spv::DecorationPatch);
        }, "require Patch float32");
        invalid(tes, [](CompilerMSL &, CompilerMSL::Options &o) { o.raw_buffer_tese_input = false; }, "require TCS or raw-buffer TES");
        invalid(tcs, [](CompilerMSL &, CompilerMSL::Options &o) { o.set_msl_version(1, 2); }, "require MSL 2.0");
        invalid(tcs, [](CompilerMSL &c, CompilerMSL::Options &) { c.set_execution_mode(spv::ExecutionModeIsolines); }, "do not support isolines");
        invalid(tcs, [](CompilerMSL &, CompilerMSL::Options &o) { o.multiview = true; }, "do not support multiview");
        invalid(tcs, [](CompilerMSL &c, CompilerMSL::Options &) { c.mask_stage_output_by_builtin(spv::BuiltInTessLevelOuter); }, "masked factors");
        invalid(tcs, [](CompilerMSL &, CompilerMSL::Options &o) { o.shader_tess_factor_buffer_index = 31; }, "[0, 30]");
        invalid(tcs, [](CompilerMSL &, CompilerMSL::Options &o) { o.shader_tess_factor_buffer_index = o.shader_output_buffer_index; }, "implicit buffer");
        invalid(tcs, [](CompilerMSL &, CompilerMSL::Options &o) { o.shader_tess_factor_buffer_index = 0; }, "application or auxiliary");
        invalid(read_spirv(argv[7]), [](CompilerMSL &, CompilerMSL::Options &) {}, "require TCS or raw-buffer TES");
        // Argument buffers only change how descriptors reach the shader; the factor buffer stays a direct float32 argument.
        for (const auto &words : {tcs, tes})
        {
            CompilerMSL compiler(words);
            configure(compiler);
            auto options = compiler.get_msl_options();
            options.argument_buffers = true;
            compiler.set_msl_options(options);
            compiler.set_execution_mode(spv::ExecutionModeTriangles);
            auto source = compiler.compile();
            bool tesc = compiler.get_execution_model() == spv::ExecutionModelTessellationControl;
            check(source.find("spvDescriptorSetBuffer0& spvDescriptorSet0") != std::string::npos, "Argument buffer was not emitted");
            check(source.find(tesc ? "device spvTessellationFactorsFloat* spvTessLevel [[buffer(26)]]" : "const device spvTessellationFactorsFloat* spvTessLevel [[buffer(26)]]") != std::string::npos, "Float32 factor buffer lost under argument buffers");
            check(source.find("half") == std::string::npos, "Float32 factors were narrowed under argument buffers");
            if (argc == 9)
            {
                std::ofstream file(std::string(argv[8]) + (tesc ? "/tcs-argument-buffers.metal" : "/tes-argument-buffers.metal"));
                file << source;
                check(bool(file), "Cannot write argument-buffer MSL");
            }
        }
        // The shape query agrees with compile() and ignores the other options.
        auto incompatibility = [&](const std::vector<uint32_t> &words, const std::function<void(CompilerMSL &)> &change) {
            CompilerMSL compiler(words);
            auto options = compiler.get_msl_options();
            options.argument_buffers = true;
            options.multiview = true;
            compiler.set_msl_options(options);
            change(compiler);
            return compiler.get_tessellation_factors_float32_incompatibility();
        };
        auto none = [](CompilerMSL &) {};
        auto oneVertex = [](CompilerMSL &c) { c.set_execution_mode(spv::ExecutionModeOutputVertices, 1); };
        check(incompatibility(tcs, none).empty() && incompatibility(tes, none).empty() && incompatibility(copy, oneVertex).empty(), "Qualifying shaders reported incompatible");
        check(incompatibility(read_spirv(argv[5]), none).find("standalone builtins") != std::string::npos, "Block factors not reported");
        check(incompatibility(read_spirv(argv[6]), none).find("explicit IO pointer") != std::string::npos, "IO pointer parameters not reported");
        check(incompatibility(read_spirv(argv[7]), none).find("require TCS or raw-buffer TES") != std::string::npos, "Vertex stage not reported");
        check(incompatibility(tcs, [](CompilerMSL &c) { c.set_execution_mode(spv::ExecutionModeIsolines); }).find("do not support isolines") != std::string::npos, "Isolines not reported");
        check(incompatibility(copy, [](CompilerMSL &c) { c.set_execution_mode(spv::ExecutionModeOutputVertices, 2); }).find("initializers currently require OutputVertices 1") != std::string::npos, "Initializers not reported");
        std::cout << "PASS: 16 TCS/raw-TES/copy variants, argument buffers, default-off option, barriers, shape query and rejection guards.\n";
    }
    catch (const std::exception &error) { std::cerr << error.what() << '\n'; return 1; }
}
