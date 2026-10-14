// Copyright 2026 Jean-Philippe Meunier
// SPDX-License-Identifier: Apache-2.0
#include "spirv_msl.hpp"
#include <algorithm>
#include <fstream>
#include <functional>
#include <iostream>
#include <sstream>
#include <stdexcept>
using namespace spirv_cross;
static void check(bool value, const std::string &message) { if (!value) throw std::runtime_error(message); }
static std::vector<uint32_t> read(const std::string &path)
{
    std::ifstream f(path, std::ios::binary | std::ios::ate);
    check(bool(f), "Cannot open " + path);
    auto n = f.tellg();
    check(n > 0 && n % 4 == 0, "Invalid SPIR-V size");
    std::vector<uint32_t> words(size_t(n) / 4);
    f.seekg(0); f.read(reinterpret_cast<char *>(words.data()), n);
    check(bool(f), "Cannot read SPIR-V");
    return words;
}
static void write(const std::string &path, const std::string &s) { std::ofstream f(path); f << s; check(bool(f), "Cannot write " + path); }
static void rejects(const std::function<void()> &f, const std::string &reason)
{
    try { f(); } catch (const CompilerError &e) { check(std::string(e.what()).find(reason) != std::string::npos, e.what()); return; }
    throw std::runtime_error("Expected rejection: " + reason);
}
static CompilerMSL::Options options(bool layered, bool compute, bool wide)
{
    CompilerMSL::Options o;
    o.set_msl_version(2, 4);
    o.multiview = true; o.multiview_layered_rendering = layered;
    o.capture_output_to_buffer = true; o.disable_rasterization = true;
    o.vertex_for_tessellation = compute;
    o.vertex_index_type = compute ? (wide ? CompilerMSL::Options::IndexType::UInt32 : CompilerMSL::Options::IndexType::UInt16) : CompilerMSL::Options::IndexType::None;
    o.shader_output_buffer_index = 28; o.indirect_params_buffer_index = 24;
    o.view_mask_buffer_index = 22; o.shader_index_buffer_index = 25; o.draw_id_buffer_index = 23;
    return o;
}
// Run the actual emitted mapping statements on CPU as well as syntax-checking the full MSL.
// This catches a compiling but incorrect application/physical instance mapping.
static std::string mapping(const std::string &msl, unsigned mode, bool layered, bool zero_base, bool effects, unsigned serial)
{
    bool compute = mode != 0;
    std::string s = "void mapping" + std::to_string(serial) + "() {\n";
    s += "for (uint F : {0u, 4u, 30u, 31u}) for (uint V : {1u, 2u, 3u, 4u, 7u, 16u, 32u}) { if (F+V>32) continue;\n";
    s += "for (int D : {-9, 0, 13}) {\n";
    // Negative vertexOffset is legal for indexed compute capture; direct capture here is nonindexed.
    if (!compute) s += "if (D < 0) continue;\n";
    s += "for (uint B : {0u, 7u}) for (uint q=0; q<3*V; ++q) for (uint x=0; x<7; ++x) {\n";
    s += "uint spvViewMask[]={F,V}; uint spvIndirectParams[]={7};\n";
    s += std::string(mode == 3 ? "uint" : "uint16_t") + " spvIndices[]={16," + (mode == 1 ? "255" : mode == 3 ? "65536" : "65535") + ",16,21,17,21,20};\n";
    s += "uint3 gl_GlobalInvocationID{x,q,0}, spvStageInputSize{7,3*V,1}, spvDispatchBase{uint(D),B,0}; uint spvDrawIndex[]={5};\n";
    s += "main0_out spvOut[672]{};\n";
    if (!compute) s += "uint gl_InstanceIndex=B+q, gl_BaseInstance=B, gl_VertexIndex=uint(D)+x, gl_BaseVertex=uint(D);\n";
    unsigned assignments[4]{};
    const char *members[] = {"appInstance", "appVertex", "appBaseInstance", "appBaseVertex"};
    std::istringstream input(msl);
    for (std::string line; std::getline(input, line); )
    {
        auto begin = line.find_first_not_of(' ');
        if (begin == std::string::npos) continue;
        auto code = line.substr(begin);
        if (code.find("device main0_out& out =") == 0) s += code.substr(7) + "\n";
        else if (code.find(" = ") != std::string::npos && (code.find("uint gl_") == 0 || code.find("const uint gl_") == 0 || code.find("gl_InstanceIndex =") == 0 || code.find("out.gl_Layer =") == 0)) s += code + "\n";
        // Copy application assignments verbatim: the oracle must not perform their base subtraction.
        for (unsigned i = 0; i < 4; ++i)
            if (code.find(std::string("out.") + members[i] + " = ") == 0) { s += code + "\n"; ++assignments[i]; }
    }
    for (unsigned i = 0; i < 4; ++i) check(assignments[i] == unsigned(i < 2 || effects), std::string("Missing/duplicate emitted application assignment: ") + members[i]);
    s += "check(&out == &spvOut[q*7+x], \"physical capture record\");\n";
    s += "check(gl_InstanceIndex == B + " + std::string(layered ? "q/V" : "q") + ", \"application instance\");\n";
    if (!compute || layered || zero_base) s += "check(gl_BaseInstance == B, \"base instance\");\n";
    s += "check(gl_ViewIndex == F + " + std::string(layered ? "q%V" : "0") + ", \"absolute view\");\n";
    s += "check(gl_VertexIndex == " + std::string(compute ? "spvIndices[x]+uint(D)" : "uint(D)+x") + ", \"base vertex and repeated indices\");\n";
    if (layered) s += "check(out.gl_Layer == q%V, \"relative layer\");\n";
    s += "check(out.appInstance == int(" + std::string(zero_base ? "" : "B+") + (layered ? "q/V" : "q") + "), \"emitted application InstanceIndex\");\n";
    s += "check(out.appVertex == int(" + std::string(compute ? "spvIndices[x]" : "x") + ")" + (zero_base ? "" : "+D") + ", \"emitted application VertexIndex\");\n";
    if (effects) s += "check(out.appBaseInstance == int(B), \"emitted application BaseInstance\"); check(out.appBaseVertex == D, \"emitted application BaseVertex\");\n";
    return s + "}}}}\n";
}
static std::string replay_mapping(const std::string &msl, uint32_t stride, uint32_t layer_offset, unsigned serial)
{
    std::string s = "void replay_mapping" + std::to_string(serial) + "() {\n";
    s += "constexpr uint stride=" + std::to_string(stride) + ", layer_offset=" + std::to_string(layer_offset) + ";\n";
    s += "alignas(16) uchar spvReplayVertices[42*stride]{}; uint spvReplayOccurrences[82]{}, spvReplayCorners[41]{};\n";
    s += "uint spvReplayDraw[]={5,11,7,6}; static_assert(sizeof(spvReplayDraw)==16, \"draw ABI\");\n";
    s += "uint corners[]={2,0,1,1,2,0}, records[]={5,1,3};\n";
    s += "for(uint q=0;q<6;++q) for(uint x=0;x<6;++x) {uint o=5+q*6+x, record=q*7+records[corners[x]];\n";
    s += "spvReplayOccurrences[2*o]=record; spvReplayOccurrences[2*o+1]=40+q*2+x/3; spvReplayCorners[o]=corners[x];\n";
    s += "*reinterpret_cast<uint*>(spvReplayVertices+record*stride+layer_offset)=q%2;}\n";
    s += "for(uint q=0;q<6;++q) for(uint x=0;x<6;++x) {uint spvReplayInstance=7+q, spvReplayVertex=11+x;\n";
    s += "struct {uint spvReplayLayer, spvReplayPrimitive;} out{};\n";
    std::istringstream input(msl);
    for (std::string line; std::getline(input, line); )
    {
        auto begin = line.find_first_not_of(' ');
        if (begin == std::string::npos) continue;
        auto code = line.substr(begin);
        if (code.find("ulong spvReplayOccurrence =") == 0 || code.find("const device uchar* spvReplayRecord =") == 0 || code.find("out.spvReplayLayer =") == 0 || code.find("out.spvReplayPrimitive =") == 0 || code.find("uint spvReplayCorner =") == 0)
        {
            for (auto pos = code.find("device "); pos != std::string::npos; pos = code.find("device ")) code.erase(pos, 7);
            s += code + "\n";
        }
    }
    s += "check(out.spvReplayLayer==q%2, \"replayed view layer\"); check(out.spvReplayPrimitive==40+q*2+x/3, \"absolute primitive key\");\n";
    return s + "check(spvReplayCorner==corners[x], \"original corner order\"); }}\n";
}
int main(int argc, char **argv)
{
    try
    {
        check(argc == 2, "Usage: test OUTPUT (containing vert/frag/effects/view-only.spv)");
        std::string dir = argv[1];
        auto frag = read(dir + "/frag.spv");
        std::string cpu = "#include <cstdint>\n#include <cstdio>\n#include <initializer_list>\n#include <stdexcept>\nusing uint=uint32_t; using ulong=uint64_t; using uchar=unsigned char; struct uint3 { uint x,y,z; }; struct main0_out { uint gl_Layer; int appInstance=-999, appVertex=-999, appBaseInstance=-999, appBaseVertex=-999; };\nvoid check(bool b,const char* m){if(!b)throw std::runtime_error(m);}\n";
        unsigned serial = 0, replay_serial = 0;
        for (const std::string fixture : {"vert", "effects", "view-only"})
        for (bool layered : {false, true})
        for (unsigned mode = 0; mode < 4; ++mode)
        for (bool zero_base : {false, true})
        {
            // mode 1 is the existing uint8 -> uint16 GPU widening ABI, mode 2 native uint16.
            bool compute = mode != 0;
            auto words = read(dir + "/" + fixture + ".spv");
            auto o = options(layered, compute, mode == 3);
            o.enable_base_index_zero = zero_base;
            CompilerMSL capture(words); capture.set_msl_options(o);
            auto msl = capture.compile(); auto layout = capture.get_msl_captured_vertex_layout();
            if (fixture == "effects") check(msl.find("atomic_fetch_add") != std::string::npos && msl.find("AppSideEffects") != std::string::npos && (msl.find("uint gl_BaseInstance = spvDispatchBase.y;") != std::string::npos) == compute, "Capture side effects and compute base instance positive control");
            std::string name = fixture + "-" + std::to_string(layered) + "-" + std::to_string(mode) + "-" + std::to_string(zero_base);
            std::string abi = "\nstatic_assert(sizeof(main0_out) == " + std::to_string(layout.stride) + ", \"capture stride\");\n";
            for (const auto &field : layout.components)
            {
                const char *members[] = {"value", "ordinary", nullptr, nullptr, nullptr, "appInstance", "appVertex", "appBaseInstance", "appBaseVertex"};
                check(field.location < 9 && members[field.location], "Unexpected capture location");
                std::string member = members[field.location];
                abi += "static_assert(__builtin_offsetof(main0_out, " + member + ") + " + std::to_string(field.component * 4) + " == " + std::to_string(field.byte_offset) + ", \"capture user offset\");\n";
            }
            for (const auto &field : layout.builtins)
            {
                std::string member = field.builtin == spv::BuiltInPosition ? "gl_Position" : field.builtin == spv::BuiltInLayer ? "gl_Layer" : "gl_ClipDistance";
                abi += "static_assert(__builtin_offsetof(main0_out, " + member + ") + " + std::to_string((field.component + field.array_index) * 4) + " == " + std::to_string(field.byte_offset) + ", \"capture builtin offset\");\n";
            }
            write(dir + "/capture-" + name + ".metal", msl + abi);
            auto layer = std::find_if(layout.builtins.begin(), layout.builtins.end(), [](const MSLCapturedVertexBuiltin &f) { return f.builtin == spv::BuiltInLayer; });
            check((layer != layout.builtins.end()) == layered, "Layer capture presence");
            if (layered) check(layer->scalar_type == SPIRType::UInt && layer->array_index == 0 && layer->component == 0 && layer->byte_offset + 4 <= layout.stride, "Layer scalar ABI");
            if (mode == 1 || mode == 2) check(msl.find("const device ushort* spvIndices") != std::string::npos || fixture == "view-only", "uint8 widening / uint16 ABI");
            if (fixture != "view-only") cpu += mapping(msl, mode, layered, zero_base, fixture == "effects", serial++);
            CompilerMSL consumer(frag);
            auto f = options(layered, false, false); f.capture_output_to_buffer = false; f.disable_rasterization = false;
            consumer.set_msl_options(f);
            consumer.set_msl_per_vertex_input_buffer(layout, {26, 25, 2});
            consumer.set_msl_fragment_barycentric_input({3, 4});
            auto fragment = consumer.compile();
            write(dir + "/fragment-" + name + ".metal", fragment);
            if (layered) check(fragment.find("[[render_target_array_index]]") != std::string::npos && fragment.find("gl_ViewIndex += spvViewMask[0]") != std::string::npos, "Fragment absolute view reconstruction");
            auto r = o; r.capture_output_to_buffer = false; r.disable_rasterization = false; r.vertex_for_tessellation = false;
            MSLCapturedOutputReplayBinding binding{0, 1, 2, 2};
            CompilerMSL guarded(words); guarded.set_msl_options(r);
            rejects([&] { guarded.compile_captured_output_replay(layout, binding); }, "does not support multiview");
#ifndef MULTIVIEW_BEFORE
            if (layered)
            {
                binding.multiview_layer = true;
                r.emulate_reversed_depth_viewport = true; r.reversed_depth_viewport_buffer_index = 5;
                CompilerMSL replay(words); replay.set_msl_options(r);
                auto common = replay.get_common_options(); common.vertex.fixup_clipspace = true; common.vertex.flip_vert_y = true; replay.set_common_options(common);
                auto source = replay.compile_captured_output_replay(layout, binding, {3, 3, 4}, {1});
                write(dir + "/replay-" + name + ".metal", source);
                if (fixture == "vert") cpu += replay_mapping(source, layout.stride, layer->byte_offset, replay_serial++);
                check(source.find("[[render_target_array_index]]") != std::string::npos, "Replay omitted synthetic layer");
                check(source.find("spvReplayRecord + " + std::to_string(layer->byte_offset) + "ul") != std::string::npos, "Replay must load captured layer offset");
                check(source.find("spvViewMask") == std::string::npos && source.find("atomic_fetch") == std::string::npos && source.find("AppSideEffects") == std::string::npos, "Replay must not execute application code or add ViewRange buffer");
                check(source.find("spvReplayCorners[spvReplayOccurrence]") != std::string::npos && source.find("spvReplayOccurrences[2ul * spvReplayOccurrence + 1ul]") != std::string::npos, "Corner/key ABI changed");
                check(source.find("[[user(locn0)") == std::string::npos && source.find("[[user(locn1)") != std::string::npos, "Selected replay changed");
                auto missing = layout; missing.builtins.erase(std::remove_if(missing.builtins.begin(), missing.builtins.end(), [](const MSLCapturedVertexBuiltin &v) { return v.builtin == spv::BuiltInLayer; }), missing.builtins.end());
                CompilerMSL bad(words); bad.set_msl_options(r);
                rejects([&] { bad.compile_captured_output_replay(missing, binding); }, "requires a captured Layer");
                for (unsigned invalid = 0; invalid < 4; ++invalid)
                {
                    auto corrupt = layout;
                    auto field = std::find_if(corrupt.builtins.begin(), corrupt.builtins.end(), [](const MSLCapturedVertexBuiltin &v) { return v.builtin == spv::BuiltInLayer; });
                    if (invalid == 0) field->scalar_type = SPIRType::Int;
                    if (invalid == 1) field->byte_offset = layout.stride;
                    if (invalid == 2) field->byte_offset = 0;
                    if (invalid == 3) corrupt.builtins.push_back(*field);
                    CompilerMSL invalid_replay(words); invalid_replay.set_msl_options(r);
                    const char *diagnostics[] = {"unsupported type", "outside", "overlapping", "duplicate builtin"};
                    rejects([&] { invalid_replay.compile_captured_output_replay(corrupt, binding); }, diagnostics[invalid]);
                }
                for (unsigned invalid = 0; invalid < 5; ++invalid)
                {
                    CompilerMSL invalid_replay(words); auto invalid_options = r;
                    if (invalid == 0) invalid_options.multiview = false;
                    if (invalid == 1) invalid_options.multiview_layered_rendering = false;
                    if (invalid == 2) invalid_options.view_index_from_device_index = true;
                    if (invalid == 3) invalid_replay.mask_stage_output_by_builtin(spv::BuiltInLayer);
                    if (invalid == 4) invalid_options.emulate_depth_clip_enable = true;
                    invalid_replay.set_msl_options(invalid_options);
                    rejects([&] { invalid_replay.compile_captured_output_replay(layout, binding); }, invalid == 4 ? "depth-clip" : "multiview");
                }
            }
#endif
            // Default non-multiview replay remains byte-comparable, even with extra captured Layer.
            r = o; r.capture_output_to_buffer = false; r.disable_rasterization = false; r.vertex_for_tessellation = false; r.multiview = false; r.multiview_layered_rendering = false;
            CompilerMSL legacy(words); legacy.set_msl_options(r);
            write(dir + "/legacy-" + name + ".metal", legacy.compile_captured_output_replay(layout, {0, 1, 2, 2}, {3, 3, 4}, {1}));
        }
#ifndef MULTIVIEW_BEFORE
        // A real application Layer must not be silently replaced by the synthetic one.
        auto app_layer = read(dir + "/application-layer.spv");
        CompilerMSL original(app_layer); auto original_options = options(false, false, false); original_options.multiview = false; original.set_msl_options(original_options);
        original.compile(); auto original_layout = original.get_msl_captured_vertex_layout();
        CompilerMSL invalid_layer(app_layer); auto invalid_options = options(true, false, false); invalid_options.capture_output_to_buffer = false; invalid_options.disable_rasterization = false; invalid_layer.set_msl_options(invalid_options);
        MSLCapturedOutputReplayBinding layer_binding{0, 1, 2, 15}; layer_binding.multiview_layer = true;
        rejects([&] { invalid_layer.compile_captured_output_replay(original_layout, layer_binding); }, "synthetic Layer");
#endif
        cpu += "int main(){try {";
        for (unsigned i=0; i<serial; ++i) cpu += "mapping" + std::to_string(i) + "();";
        for (unsigned i=0; i<replay_serial; ++i) cpu += "replay_mapping" + std::to_string(i) + "();";
        write(dir + "/mapping.cpp", cpu + "} catch(const std::exception& e){std::fprintf(stderr, \"%s\\n\", e.what()); return 1;}}\n");
#ifdef MULTIVIEW_BEFORE
        std::cout << "BASELINE: generated capture/fragment/default replay; opt-in replay is not enabled\n";
#else
        std::cout << "PASS: capture, fragment, guarded/default replay, layered replay and ABI checks\n";
#endif
    }
    catch (const std::exception &e) { std::cerr << e.what() << '\n'; return 1; }
}
