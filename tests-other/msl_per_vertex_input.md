# Portable captured PerVertexKHR input ABI

This is a compiler-side subset, not a runtime capture/replay system or a claim of GPU support on all Macs. Configure `CompilerMSL` before compilation. The C++ API is intended for callers such as MoltenVK; no C API or CLI layout syntax is added.

## Selecting the path

- Default: no PerVertexKHR support is enabled.
- Native: `Options::supports_per_vertex_fragment_input = true`, MSL >= 4.0, and the caller has verified Apple10+ `vertex_value` support and the native triangle contract. MSL version alone is insufficient.
- Portable: call `set_msl_per_vertex_input_buffer(layout, binding)`, use MSL >= 2.4, and leave the native option false. This path never introduces `vertex_value` or `[[primitive_id]]`.
- Enabling both paths for active PerVertexKHR inputs throws. No automatic hardware selection occurs.
- `needs_per_vertex_input_buffer()` after successful compilation tells the caller whether these resources are required. An enabled portable configuration with no active PerVertexKHR input adds no resources and does not change generated MSL. Existing builtin feature requirements remain applicable.

## Producer and resource contract

`MSLCapturedVertexLayout::stride` is the actual byte stride of a captured producer record. Each `MSLCapturedVertexComponent` specifies `(location, component, byte_offset, scalar_type)` for one physical scalar. Offsets are relative to the record start. Components may be noncontiguous; vector padding, unused outputs, and holes are not inferred from the fragment shader. A normal Metal `float3` may occupy 16 bytes even though only 12 bytes are read. A packed or reordered producer requires its own correct offsets and stride. For a producer compiled by this backend, `get_msl_captured_vertex_layout()` exports that physical layout after successful compilation (see below). Externally generated producers still require an explicit matching layout.

### Exporting the compiled producer layout

After compiling a vertex shader with `Options::capture_output_to_buffer = true`, call `get_msl_captured_vertex_layout()`. Its value can be passed directly to the fragment's `set_msl_per_vertex_input_buffer()`:

```cpp
auto vertex_options = vertex_compiler.get_msl_options();
vertex_options.set_msl_version(2, 4);
vertex_options.capture_output_to_buffer = true;
vertex_compiler.set_msl_options(vertex_options);
std::string vertex_msl = vertex_compiler.compile();
spirv_cross::MSLCapturedVertexLayout layout = vertex_compiler.get_msl_captured_vertex_layout();
fragment_compiler.set_msl_per_vertex_input_buffer(layout, binding);
std::string fragment_msl = fragment_compiler.compile(); // Configure fragment MSL >= 2.4 beforehand.
```

The getter reads the **final emitted record**, including member sorting, physical type remapping, member alignment, emitted padding, and tail padding. It reuses the MSL size/alignment helpers; SPIR-V offsets alone are not the capture ABI. Builtins occupy space and affect stride but are **not** returned as Location/Component fields, even if a builtin has an interface location remap. Masked outputs are absent. For Component-packed vectors, only declared lanes are returned; inserted lanes are padding, not additional producer outputs. Query before changing compiler configuration/IR. A failed compilation invalidates the query; no partial layout is returned on any error.

Supported export: final physical numeric scalar/vector user members of 16/32 bits, including flattened interface-block members, plus numeric literal-size one-dimensional builtin arrays for replay. A retained user array, matrix, nested struct, pointer, opaque/64-bit field, unresolved builtin-array size, ambiguous location, duplicate field, omitted/overlapping physical member or overflowing uint32 offset/stride is rejected. A composite already flattened by the backend into supported members is described according to those final members; the portable consumer can reconstruct composite PerVertex fragment inputs from matching scalar descriptors. The getter is restricted to vertex capture with an output record, including the vertex-as-compute mode. It does not export TCS/TES/mesh or patch records.

The simple regression exports stride **32** and color offsets **0/4/8** for `float3 color` plus Position. The mixed regression exports stride **112**, with component-packed `float4`, `half3`, `ushort`, a flattened block (`float`, `float3`), another `float3`, Position, PointSize, a two-float ClipDistance array, Layer and ViewportIndex. C++ tests check the expected ABI and append `sizeof`, `__builtin_offsetof`, and type assertions to the **actual generated MSL**, compiled with Metal. Builtin arrays with stage-output attributes in vertex capture records use native arrays, since Metal rejects `clip_distance` on the generic array wrapper. Default/native arrays, vertex/vertex-as-compute capture, builtin location remapping, and forwarding ClipDistance to a helper function are tested.

`layout.builtins` contains `MSLCapturedVertexBuiltin` scalar descriptors keyed by `(builtin, array_index, component)`, with `byte_offset` and `scalar_type`. Scalar builtins use both indices zero. Position uses `array_index = 0`, components 0..3; ClipDistance uses its array index with component zero. Each offset is absolute within the captured record. Types describe the emitted Metal representation: for example Layer and ViewportIndex are `UInt`, even when the source SPIR-V uses signed int. Builtins remain separate from the application Location/Component namespace, including when a builtin is assigned a linkage location. `set_msl_per_vertex_input_buffer()` ignores this replay metadata and reads only `components`.

Replay reconstructs a builtin using `load<field.scalar_type>(captured + uint64(record) * layout.stride + field.byte_offset)` for each of its scalar descriptors. The simple fixture verifies Position at offsets **16/20/24/28**. The complex fixture verifies Position at **64/68/72/76**, PointSize at **80**, ClipDistance[0..1] at **84/88**, Layer at **92**, and ViewportIndex at **96**. All are checked against the actual MSL type with compile-time assertions; masked builtins have no descriptor. The separate `compile_captured_output_replay()` API uses this metadata to generate a vertex replay shader without executing application SPIR-V. The caller still selects which builtins the render pipeline permits/needs.

The runtime must still implement the producer's capture index formula and construct the primitive table against those actual record indices. A correct layout does not establish draw/instance/topology mapping or memory visibility. Include both producer compilation options and the exported layout in the existing pipeline/cache compatibility decisions.

The masking regression covers PointSize and ClipDistance. Masking Layer on the mixed fixture currently emits an undeclared `gl_Layer` assignment in the capture backend; that configuration is not validated for integration. This getter describes the generated record, and does not certify that every backend option combination produces legal Metal. The implemented vertex replay compiler subset is described separately in [msl_captured_output_replay.md](msl_captured_output_replay.md).

`MSLPerVertexInputBinding` specifies:

- `vertex_buffer_index`: `const device uchar*` containing captured records.
- `primitive_index_buffer_index`: `const device uint*` containing exactly three consecutive uint32 record indices per primitive (12 bytes per entry). **Do not use an array of Metal uint3**, which has a different stride.
- `primitive_index_location`: reserved `user(locnN)` interface location. The replay/rasterizing shader must output a `uint` varying with the same value at every vertex of each primitive. Its integral interpolation is flat in Metal. SPIRV-Cross adds the matching fragment member.

For fragment key `K` and local vertex `v`:

```text
record = indices[uint64(K) * 3 + v]
scalar = load<scalar_type>(vertices + uint64(record) * stride + byte_offset)
```

`K` is an **absolute triplet index in the bound table**, not a byte offset, uint element offset, hardware PrimitiveId, or instance ID. The caller includes draw/instance/view identity in K and in the record indices. There is no separate instance offset argument. Binding offsets may select a draw's table/capture slice; K and indices are relative to those bound slices. Both bound bases must be aligned to at least four bytes. The table and every accessed scalar must be within the bound buffers. The compiler uses 64-bit address arithmetic to avoid uint32 multiplication wraparound; this does not enable 64-bit shader input types. Bounds, initialization, lifetime and memory visibility are the caller's responsibility.

Example for a producer whose `float3 color` occupies bytes 0-11 of a 32-byte record:

```cpp
spirv_cross::MSLCapturedVertexLayout layout;
layout.stride = 32;
for (uint32_t component = 0; component < 3; component++)
{
    spirv_cross::MSLCapturedVertexComponent field;
    field.location = 0;
    field.component = component;
    field.byte_offset = 4 * component;
    field.scalar_type = spirv_cross::SPIRType::Float;
    layout.components.push_back(field);
}
spirv_cross::MSLPerVertexInputBinding binding;
binding.vertex_buffer_index = 28;
binding.primitive_index_buffer_index = 29;
binding.primitive_index_location = 15;
auto options = compiler.get_msl_options();
options.set_msl_version(2, 4);
options.supports_per_vertex_fragment_input = false;
compiler.set_msl_options(options);
compiler.set_msl_per_vertex_input_buffer(layout, binding);
std::string msl = compiler.compile();
```

MoltenVK must reserve both fragment buffer slots and the private interface location, supply the physical layout from the actual final producer, and include this configuration in shader/pipeline cache identity. `is_msl_shader_input_used()` still reports captured source locations; the private location is not exposed as an application input. The replay shader need not interpolate those captured inputs to the fragment. The fragment setter does not modify the producer or replay shader; compile those separately with their corresponding APIs.

The runtime owns primitive assembly and ordering, base vertex/index handling, restart, degenerates, provoking vertex rules, instance/view selection, and point/line duplication into triplets. Shared indexed vertices may need separate replay occurrences to carry distinct primitive keys. Capture must occur at the last relevant vertex-producing stage. Replay must not repeat application shader side effects. Writes must be visible before fragment reads, including the requirements of the active render pass and transient attachments. None of these runtime properties is proven by syntax tests.

## Supported subset and validation

Supported: variable-decorated PerVertexKHR arrays of 1-3 vertices. Each vertex may contain matrices, literal-size arrays (including multidimensional arrays), structs and blocks, recursively composed of scalars/vectors of `Half`, `Short`, `UShort`, `Float`, `Int` or `UInt`; scalar-exact producer types; Location/Component; dynamic indexing; forwarding local arrays to functions; ordinary inputs; classic bindings and argument buffers; SPIRV-Cross arrays and native MSL arrays. Existing arrays are reconstructed before the shader body; the ordinary access-chain path is preserved.

Rejected: member-decorated PerVertexKHR (see the specification boundary below), builtin members, nested PerVertexKHR decorations, nonliteral or zero array sizes, 64-bit or other scalar types, missing locations/mappings, mismatched scalar types, captured input remapping through `add_msl_shader_input`, duplicate logical fields, overlapping physical scalars, misaligned or out-of-stride scalars, invalid or colliding resource indices, private location collisions, and fragment output capture. Extra producer scalar fields are allowed but must satisfy the same layout rules. Buffer indices must be distinct and in [0, 30]; collision checks include buffer arrays, actual argument buffer bindings and implicit fragment buffers. The runtime must also reserve Metal's vertex descriptor buffer slots where applicable.

No GPU execution, image comparison, pipeline linking on hardware, topology validation or performance measurement is part of this test. In particular, compiling MSL 2.4 is not proof of runtime portability, synchronization or ordering across all devices.

## Runnable CPU checks

`spirv-cross-msl-per-vertex-input-test` is registered with CTest when the existing SPIRV-Cross test prerequisites are enabled. It consumes the checked-in `msl_per_vertex_input.spv`; regenerate it with:

```sh
glslangValidator -V --target-env vulkan1.1 tests-other/msl_per_vertex_input.frag -o tests-other/msl_per_vertex_input.spv
spirv-val --target-env vulkan1.1 tests-other/msl_per_vertex_input.spv
ctest --test-dir BUILD -R '^spirv-cross-msl-per-vertex-input-test$' --output-on-failure
```

The executable can also write four MSL variants (classic/argument buffers, default/native arrays):

```sh
mkdir -p OUT
BUILD/spirv-cross-msl-per-vertex-input-test tests-other/msl_per_vertex_input.spv OUT
for shader in OUT/*.metal; do
    xcrun metal -x metal -std=macos-metal2.4 -fsyntax-only -fno-modules "$shader" || exit
done
```

An optional third argument loads the standalone single-color PerVertex reproducer and uses its explicit 32-byte producer layout. Unsupported fixtures can be checked with `TEST fixture.spv --reject 'expected diagnostic substring'`. Use the existing 64-bit fixture to exercise unsupported scalar types. Optimizing the typed fixture with `spirv-opt -O` and rerunning the test exercises the same ABI after optimizer transformations.

Producer export regressions are registered as `spirv-cross-msl-capture-layout-test`. Regenerate each `msl_capture_layout_*.spv` from the corresponding `.vert`/`.frag` source with the same `glslangValidator -V --target-env vulkan1.1` invocation. Run both tests with:

```sh
ctest --test-dir BUILD -R '^spirv-cross-msl-(capture-layout|per-vertex-input)-test$' --output-on-failure
BUILD/spirv-cross-msl-capture-layout-test tests-other/msl_capture_layout_simple.spv tests-other/msl_capture_layout_complex.spv tests-other/msl_capture_layout_unsupported.spv tests-other/msl_capture_layout_consumer.spv OUT
```

The output-directory argument writes producer MSL with compile-time layout assertions and consumer MSL configured directly with the exported producer layout. Compile those outputs with the MSL 2.4 command above. No GPU execution is involved.

## Composite inputs and specification boundary

The portable path reconstructs the original local composite, so dynamic access chains and helper-function arguments retain their original shape. Only the outer vertex dimension is removed for Location assignment. Inner arrays consume successive element locations, matrices consume successive column locations, and structs are traversed in declaration order. A block may use a Location on the variable or individual Location/Component decorations on its top-level members. The physical layout remains an explicit producer contract; this change does not broaden the producer exporter's accepted output shapes.

Normative sources checked on 2026-09-26:

- [SPV_KHR_fragment_shader_barycentric, Decoration 5285](https://github.com/KhronosGroup/SPIRV-Registry/blob/main/extensions/KHR/SPV_KHR_fragment_shader_barycentric.asciidoc): PerVertexKHR may decorate a memory object or a structure member, Input only.
- [Vulkan SPIR-V environment](https://github.com/KhronosGroup/Vulkan-Docs/blob/main/appendices/spirvenv.adoc): VUID 06777 requires fragment Input; 06778 requires an array. These rules do not impose scalar/vector-only elements.
- [Vulkan interfaces](https://github.com/KhronosGroup/Vulkan-Docs/blob/main/chapters/interfaces.adoc), User-Defined Variable Interface, Interface Matching, and Location and Component Assignment: recursively numeric composites are allowed; the extra vertex array level is disregarded for matching and location assignment; inner arrays, matrices and structs use the recursive location rules. Numeric 16-bit interfaces require storageInputOutput16; 64-bit numeric interfaces are legal under their applicable features but remain outside this portable implementation's 16/32-bit ABI.
- [GL_EXT_fragment_shader_barycentric](https://github.com/KhronosGroup/GLSL/blob/main/extensions/ext/GLSL_EXT_fragment_shader_barycentric.txt), sections 4.3.4 and 4.5: an unsized vertex array becomes size three, explicit sizes must be no more than three. The compiler retains its existing 1-3 limit; this GLSL rule is not asserted to be a separate SPIR-V maximum-size VUID.

**Member Location/Component is supported; member PerVertexKHR is still rejected.** Although SPIR-V permits the latter decoration, Vulkan's special array-element matching rule explicitly says “neither the input nor the output is a structure member.” Therefore a member array cannot be assumed to match a producer scalar/vector by stripping its outer dimension. The current scalar layout API does not encode a separate interpretation for that case. No speculative member ABI is implemented, and this does not claim that every possible use of the SPIR-V member decoration is forbidden.

The explicit `msl_per_vertex_input_members.spvasm` fixture records this boundary. It passes standalone `spirv-val`, which does not prove a linked producer/consumer interface or authorize stripping member array dimensions. Current glslang also drops `pervertexEXT` on individual GLSL block members, so GLSL alone is not a sufficient fixture for that decoration. The rejection test preserves the existing diagnostic.

Regenerate and run the focused fixtures:

```sh
glslangValidator -V --target-env vulkan1.1 tests-other/msl_per_vertex_input_composites.frag -o tests-other/msl_per_vertex_input_composites.spv
spirv-as --target-env vulkan1.1 tests-other/msl_per_vertex_input_members.spvasm -o tests-other/msl_per_vertex_input_members.spv
spirv-val --target-env vulkan1.1 tests-other/msl_per_vertex_input_composites.spv
spirv-val --target-env vulkan1.1 tests-other/msl_per_vertex_input_members.spv
ctest --test-dir BUILD -R 'spirv-cross-msl-(per-vertex|capture)' --output-on-failure
BUILD/spirv-cross-msl-per-vertex-input-test tests-other/msl_per_vertex_input_composites.spv --composites OUT
```

The C++ checks use deliberately reversed scalar offsets to verify exact leaf/vertex mapping, matrix columns, nested arrays, nested structs, block member locations and component packing, 1/2/3 vertex arrays, ordinary interpolated input coexistence, location reflection, missing mappings and remapping rejection on later columns. Four generated MSL variants exercise classic/argument buffers and default/native arrays. The same checks run after `spirv-opt -O`. Syntax checks use the MSL 2.4 invocation above; no GPU or cross-stage linkage claim follows from them.
