# Captured output replay compiler

The C++ `CompilerMSL::compile_captured_output_replay()` API emits a normal rasterizing vertex shader which reads captured output without executing the application shader. The supported subset and checks below are implemented and tested on CPU and with Metal syntax compilation; this is not a complete runtime capture/replay system. The producer getter and portable fragment API are documented in [msl_per_vertex_input.md](msl_per_vertex_input.md). No C API or CLI entry point is added.

## API and compilation state

C++ entry point on a **fresh** `CompilerMSL`, constructed from the original vertex SPIR-V:

```cpp
struct MSLCapturedOutputReplayBinding
{
    uint32_t vertex_buffer_index = ~0u;
    uint32_t occurrence_buffer_index = ~0u;
    uint32_t draw_parameters_buffer_index = ~0u;
    uint32_t primitive_index_location = ~0u;
    bool multiview_layer = false;
};

std::string compile_captured_output_replay(const MSLCapturedVertexLayout &layout, const MSLCapturedOutputReplayBinding &binding);

struct MSLCapturedOutputReplayBarycentricBinding
{
    uint32_t corner_buffer_index = ~0u;
    uint32_t perspective_location = ~0u;
    uint32_t no_perspective_location = ~0u;
};

std::string compile_captured_output_replay(const MSLCapturedVertexLayout &layout, const MSLCapturedOutputReplayBinding &binding, const MSLCapturedOutputReplayBarycentricBinding &barycentrics);
```

Configure the original entry point, specialization constants, output masks/remaps and rasterization options before calling. MSL must be at least 2.4. Capture, vertex-as-compute and disabled rasterization must be off for replay. The layout comes from the successfully compiled capture producer. Preserve a separate unmodified IR/configuration for replay: ordinary `compile()` mutates IR, builds and sorts interface structs, localizes globals and installs application fixup hooks. The method rejects a compiler on which compilation has already started, and consumes its own compiler even on failure. A subsequent ordinary `compile()` is also rejected. The capture-layout getter remains specific to successful ordinary capture compilation, not replay.

The generated function builds the ordinary rasterizing vertex stage-out interface through existing MSL backend interface logic, loads each required user output and builtin from the captured record, and adds one private `uint [[user(locnN), flat]]` key. It preserves the ordinary VS's linkage locations/components, physical stage types, builtin attributes and output masks. Application interpolation follows the ordinary MSL backend convention: qualifiers are selected on the fragment interface, not added to vertex outputs. The tests compare the emitted normal/replay interfaces with flat and noperspective source outputs. The private key is explicitly flat. Application SPIR-V function bodies, resources, input attributes and side effects are not emitted. Backend clip-space/Y rasterization fixups are applied after the loads.

The captured struct cannot simply be returned unchanged. Its members can be packed differently (for example Component=2 vec2 captured in a float4), and capture disables normal rasterization paths. The scalar layout does not contain interpolation qualifiers or the complete raster interface; original SPIR-V and matching interface configuration are required inputs. Any mismatch or missing required field must fail compilation, not silently zero the output.

## Buffer ABI

All buffer indices are explicit, distinct Metal slots in [0, 30]. By default the only emitted resources are these three classic buffers, even if application argument buffers are enabled; application binding slots can be reused because their resources are absent. Reversed-depth viewport emulation optionally adds the mask buffer described below. Bind buffer bases with alignment at least 4. `primitive_index_location` must be unused in both the captured layout and raster interface, and must equal the portable fragment binding's private location.

| Resource | Representation |
| --- | --- |
| Captured records | Read-only bytes; `layout.stride` and scalar offsets/types from the producer getter. |
| Replay occurrences | Read-only tightly packed uint32 pairs: `(absolute_record_index, absolute_primitive_key)`, **8 bytes** per occurrence. |
| Draw parameters | Four uint32 scalars, **16 bytes**: `(occurrence_base, vertex_id_origin, instance_id_origin, occurrences_per_instance)`. |

When `CompilerMSL::Options::emulate_reversed_depth_viewport` is true, set the existing `Options::reversed_depth_viewport_buffer_index` to the desired mask slot (the existing option default is 18). Replay emits `constant uint& spvEmulatedReversedDepthViewportMask [[buffer(N)]]`, matching the ordinary VS: one **4-byte uint32** mask, bit `v` indicating reversed depth for viewport `v`. The slot must be in [0, 30] and distinct from the captured-record, occurrence and draw-parameter slots; collisions and out-of-range values (including `~0u`) throw `CompilerError`. No automatic slot allocation or additional replay binding field is introduced. When emulation is false, the mask index is ignored, no mask argument or correction is emitted, and the existing three-buffer ABI and generated source are unchanged.

With Metal's actual `vertex_id` and `instance_id`, use unsigned 64-bit address arithmetic:

```text
v = uint64(vertex_id)   - vertex_id_origin
i = uint64(instance_id) - instance_id_origin
occurrence = occurrence_base + i * uint64(occurrences_per_instance) + v
record = occurrences[2 * occurrence + 0]
key    = occurrences[2 * occurrence + 1]
scalar_address = captured + uint64(record) * layout.stride + scalar.byte_offset
```

The caller guarantees IDs are at least their origins, address arithmetic does not overflow, all addresses are in bounds, and table rows are initialized. Origins normalize the **replay draw**, not the original application's vertex indices. The runtime may use an expanded nonindexed replay starting at zero; then origins are zero. Single-instance draws may use `occurrences_per_instance = 0`. Instanced draws use an explicit occurrence block per instance, or deliberately share a block if records and keys are identical. Draw parameters are runtime data, not shader specialization/cache constants.

`key` is exactly the portable fragment ABI's absolute triplet index: fragment vertex `j` reads `primitive_indices[3 * uint64(key) + j]`. There is no inferred primitive division, index topology, base vertex, instance or view offset. All replay vertices of one primitive must carry the same key. Shared indexed vertices generally require separate occurrences per primitive to satisfy this; the backend does not create that expansion. The occurrence table also makes the record mapping independent of whether capture used a vertex function or a compute kernel.

## Optional triangle barycentric producer

The three-argument overload matches the [portable fragment barycentric opt-in](msl_barycentric_portable.md) from `66ad847b`. The existing two-argument API and `MSLCapturedOutputReplayBinding` are unchanged. Passing a default-constructed third binding also preserves the legacy source and ABI exactly. Opt in by setting one or both locations and a corner buffer:

```cpp
MSLCapturedOutputReplayBarycentricBinding barycentrics;
barycentrics.corner_buffer_index = 3;
barycentrics.perspective_location = 12;
barycentrics.no_perspective_location = 14;
std::string msl = replay.compile_captured_output_replay(layout, binding, barycentrics);
```

Use the same locations in `MSLFragmentBarycentricInputBinding`; leave an unused producer location at `~0u`. This does not turn on fragment barycentrics or PerVertex input automatically. Reserve each supplied location exclusively, distinct from the other basis location, primitive key, all captured user fields (including masked fields and Component-packed fields), raster outputs and `add_msl_shader_output` mappings. The producer cannot inspect the fragment: the caller must link matching locations and include the opt-in, buffer slot and both locations in shader/pipeline cache identity.

The additional resource is `const device uint* spvReplayCorners [[buffer(N)]]`: tightly packed **uint32 scalars, 4 bytes per occurrence**, with base alignment at least 4. Its slot must be in [0, 30] and distinct from all three replay buffers and the reversed-depth mask when enabled. A location without a valid corner slot, a slot without any location, duplicate locations or collisions fail compilation. Buffer contents and draw-dependent bounds remain the caller's responsibility, as with the existing occurrence and record buffers.

The original 8-byte occurrence pairs and 16-byte draw parameters retain their representation and addressing. Corners use that **same absolute occurrence index**, including occurrence base and normalized vertex/instance origins:

```text
corner = corners[occurrence]           // 0, 1 or 2 in original PerVertexKHR order
basis  = float3(corner == 0, corner == 1, corner == 2)
```

Each configured output is `float3 [[user(locnN)]]` and receives this identical unscaled basis. Fragment interpolation selects perspective versus no-perspective and center/centroid/sample evaluation. Replay neither scales by Position.w nor modifies any existing captured scalar load, primitive key or position fixup. It never derives the corner from vertex ID, record index, primitive key or modulo arithmetic. An out-of-range corner yields a zero vector through the comparisons, not an out-of-range vector write; it is invalid input and does not produce meaningful barycentrics.

For example, if primitive key 40 has the original record triplet `(4, 7, 9)` and key 41 has `(9, 6, 12)`, a valid occurrence slice is:

| Absolute occurrence | Record | Primitive key | Corner | Basis |
| --- | --- | --- | --- | --- |
| 5 | 7 | 40 | 1 | `(0, 1, 0)` |
| 6 | 9 | 40 | 2 | `(0, 0, 1)` |
| 7 | 4 | 40 | 0 | `(1, 0, 0)` |
| 8 | 9 | 41 | 0 | `(1, 0, 0)` |
| 9 | 6 | 41 | 1 | `(0, 1, 0)` |
| 10 | 12 | 41 | 2 | `(0, 0, 1)` |

Record 9 is reused with different keys and corners. Both tables must contain the required prefix through occurrence 5; setting a nonzero draw occurrence base does not make corner indexing relative to that base. In portable PerVertex mode the invariant is `record == primitive_indices[3 * uint64(key) + corner]`. Use distinct replay vertex occurrences whenever a shared source vertex needs different keys or corners; shared replay indices cannot represent that change. The runtime still owns triangle assembly, winding/provoking-vertex conventions, strips/fans/restarts, degenerates, clipping compatibility and instance/view selection. This is a triangle contract, not point/line barycentric support or an automatic topology expansion.

## Fixups and initial limits

Capture skips `CompilerMSL::emit_fixup()`, including clip-space conversion, Y inversion, default PointSize and depth/viewport emulation. Replay calls this same raster fixup emitter **exactly once** after loading captured outputs: optional `Options::vertex.fixup_clipspace`, optional reversed-depth correction, then optional `Options::vertex.flip_vert_y`. Reversed-depth correction is `Position.z = Position.w - Position.z` when the selected mask bit is set. It uses `uint(ViewportIndex)` from the captured output if present, otherwise bit zero (also when ViewportIndex is masked). The caller must supply a device-valid viewport index below 32 and bind the same mask as an equivalent ordinary VS draw; no index clamping or mask inference is performed. This supports the existing emulation path on older AMD Mac2 without requiring MSL 4.0. It never invokes application output hooks. `emulate_depth_clip_enable` and `enable_point_size_default` remain explicitly rejected. Multiview requires the explicit captured-layer mode below. A captured application PointSize is supported; missing outputs are not silently synthesized.

Implemented subset:

- Original vertex entry point; no tessellation, geometry, mesh, patch records, automatic multiview or layered-rendering emulation. Exporting a vertex-as-compute record does not imply tessellation replay support.
- User scalar/vector physical members of Half/Short/UShort/Float/Int/UInt, recursively contained in structs, literal nonzero arrays (including multidimensional arrays and arrays of structs/matrices), and matrices. Explicit sparse member Locations and scalar/vector array Components are preserved. Capture records retain their composite declarations; replay emits scalar/vector raster members. Nonliteral dimensions are rejected before interface flattening; pointers, 64-bit and other scalar types, and packed or stride-remapped structs remain unsupported and fail layout export.
- Position (required), PointSize, ClipDistance, Layer and ViewportIndex, with explicit type/shape checks and pipeline/device feature checks by the caller. Other active output builtins or builtin descriptors are rejected even if masked. ClipDistance's raster array and optional `user(clipN)` aliases are all loaded from the same captured builtin descriptors.
- Layout validation includes required user/builtin coverage, duplicate keys, offset bounds/alignment, physical type compatibility, overlapping user/builtin scalars, buffer/varying collisions and supported rasterization options. Extra valid captured fields can be ignored. Diagnostics identify missing user Location/Component or builtin/index/component; no partial shader is returned on failure.
- Separate namespaces for user fields and builtins. Load each builtin via `(builtin, array_index, component)`; Layer/ViewportIndex use their exported physical UInt types.

The runtime owns topology/restart/degenerate handling, provoking vertex order, original capture index formulas, instance/view mapping, bounds, synchronization, lifetime and final pipeline compatibility. It must ensure application side effects happen only during capture. Values not written by the original shader remain undefined. This compiler subset does not establish runtime portability on every MSL 2.4 device. In particular, syntax validation is not pipeline validation for PointSize, Layer or ViewportIndex.

## CPU and Metal checks

`spirv-cross-msl-capture-replay-test` is a native C++ project target registered with CTest. It checks simple/complex capture, normal raster and replay shaders with both native/default arrays and vertex/compute capture. Interfaces are compared after removing the private key. The side-effect fixture has a positive control atomic SSBO write in capture, absent from replay along with the application resource and helper. It also covers masks, builtin remapping, ClipDistance aliases, all four clip-space/Y combinations, and invalid layouts/bindings/options/compiler reuse/stages. Reversed-depth cases compare ordinary VS and replay mask arguments, guards, correction count and fixup order for absent and captured ViewportIndex, with vertex/compute capture and all clip-space/Y combinations. A separate replay check masks ViewportIndex from a complete captured layout and verifies viewport-zero selection. Capture must emit neither the mask nor raster fixups. Slot 30 proves explicit non-default binding; all three collisions, slot 31 and `~0u` must fail only when emulation is enabled. Disabled variants must match the default replay source exactly.

```sh
glslangValidator -V --target-env vulkan1.1 tests-other/msl_capture_replay_effects.vert -o tests-other/msl_capture_replay_effects.spv
glslangValidator -V --target-env vulkan1.1 tests-other/msl_capture_replay_unsupported.tese -o tests-other/msl_capture_replay_unsupported.spv
cmake --build BUILD --target spirv-cross-msl-capture-replay-test spirv-cross-msl-capture-layout-test spirv-cross-msl-per-vertex-input-test
ctest --test-dir BUILD -R '^spirv-cross-msl-(capture-replay|capture-layout|per-vertex-input)-test$' --output-on-failure
mkdir -p OUT
BUILD/spirv-cross-msl-capture-replay-test tests-other/msl_capture_layout_simple.spv tests-other/msl_capture_layout_complex.spv tests-other/msl_capture_replay_effects.spv tests-other/msl_capture_replay_unsupported.spv tests-other/msl_capture_layout_unsupported.spv OUT
for shader in OUT/*.metal; do
    xcrun metal -x metal -std=macos-metal2.4 -fsyntax-only -fno-modules "$shader" || exit
done
```

The test writes 134 MSL shaders: the original 98 capture, normal raster, replay and reversed-depth variants, plus 36 barycentric replay variants (three producer fixtures, perspective/no-perspective/both outputs, four clip-space/Y combinations, all with reversed-depth emulation). The optional interface and basis are removed from each generated source and the remaining source is compared exactly with legacy replay, checking all scalar loads, addressing, the key, Position including w, side-effect exclusion and fixups. Tests also check private reflection, disabled defaults, explicit occurrence corner reads, and invalid bindings, including raster-only and Component mapping collisions.

Run the focused replay API and all generated Metal syntax checks without GPU work:

```sh
bash tests-other/msl_capture_replay.sh BUILD OUT
```

These are CPU code-generation and MSL 2.4 syntax checks; they do not establish AMD Mac2 pipeline creation, interpolation accuracy or GPU replay execution. MoltenVK integration remains separate validation work. TES is a rejection fixture, not a supported replay stage.

## Opt-in captured multiview layer

Set `MSLCapturedOutputReplayBinding::multiview_layer = true` with both `Options::multiview` and `Options::multiview_layered_rendering` enabled. `view_index_from_device_index` must be false. The original vertex SPIR-V may read ViewIndex without declaring Layer: replay adds a uint `[[render_target_array_index]]` member and loads it through the existing validated builtin layout descriptor `(Layer, 0, 0)`. The captured value is **relative to this pass's attachment base view**, not an absolute Vulkan ViewIndex. Replay adds no ViewRange buffer, no draw constants and no application execution. Fragment multiview continues to reconstruct absolute ViewIndex by adding `ViewRange[0]` to its layer input.

A missing, signed, out-of-bounds, overlapping or duplicate Layer descriptor fails compilation. An application-written Layer, a masked Layer or incompatible multiview options also fail. Existing unsupported-stage, buffer-slot, private-location, shape and raster-fixup guards still apply. Selection of user locations, original corner tables, both barycentric bases and position/clip/depth fixups compose with this mode. Default `multiview_layer = false` preserves the previous replay source/ABI and rejection of multiview. Ordinary non-multiview replay of an application Layer remains supported.

This bool is a C++ compiler configuration field, not a GPU buffer field. Callers must include it (and the existing multiview options/layout/bindings) in converter equality, pipeline/cache identity and serialization before enabling runtime support. No C API/CLI flag, MoltenVK converter flag, cache migration or runtime admission change is supplied here.

For a consecutive view group beginning at `F`, with `V > 0` views, `I` application instances and `N` original input occurrences:

```text
q = application_instance_ordinal * V + local_view
record = q * N + occurrence
application InstanceIndex = firstInstance + q / V
application ViewIndex = F + q % V
captured Layer = q % V
```

Direct capture receives Metal `instance_id = firstInstance + q` and `base_instance = firstInstance`; it binds the physical output record **before** remapping InstanceIndex. VertexIndex retains the draw's firstVertex. Indexed vertex-as-compute uses `thread_position_in_grid.y = q`, `grid_origin.y = firstInstance`, `grid_size.x = N`, and `grid_origin.x = uint32(vertexOffset)` (including signed negative offsets). View and application instance mapping depend directly on grid y, independently of SPIR-V variable declaration order. VertexIndex remains `indices[grid.x] + grid_origin.x`. Compute base builtins come from grid origin, never raster `[[base_instance]]` / `[[base_vertex]]` arguments. `enable_base_index_zero` normalizes application reads only, after view expansion is removed. Physical record addressing stays unchanged.

The uint8 path remains the existing runtime **GPU widening to uint16** followed by `IndexType::UInt16`; SPIRV-Cross does not add a native Metal uint8 indexed stage-input pipeline. UInt16 and UInt32 retain their existing pointer types. No CPU index readback is introduced.

The caller must expand capture dispatch/draw counts, storage, index/occurrence/key/corner tables and attribute step/divisor configuration over q. Restart compaction must retain the original per-instance `N` stride. Sparse masks are partitioned into consecutive groups: e.g. mask `0x33` gives `(F,V)=(0,2),(4,2)`; absolute view bits must not index dense capture storage. All vertices of a replay primitive must select records from the same view and carry the same absolute primitive key. Occurrence/corner addressing, replay origins and buffer layouts remain the original 8-byte pair / 4-byte corner / 16-byte draw ABI. `V`, counts, buffer bounds and products are runtime responsibilities; this compiler does not infer or allocate them. In particular, reservation reuse, multipass scheduling, attribute fetch/divisors and cache integration still require MoltenVK work before multibit admission can change.

### Reproducible compiler-only checks

Build the MSL library, then run:

```sh
cmake -S . -B build/multiview-prerequisites/compiler -G Ninja -DCMAKE_BUILD_TYPE=Release
cmake --build build/multiview-prerequisites/compiler --target spirv-cross-msl -j 4
bash tests-other/msl_multiview_capture.sh build/multiview-prerequisites/compiler build/multiview-prerequisites/after
```

The script compiles three real Vulkan 1.1 vertex variants (the original reproducer, application side effects/base builtins/ClipDistance, ViewIndex-only) and a PerVertexKHR/both-barycentric fragment with glslang, and validates all SPIR-V with `spirv-val`. It tests direct, widened uint8, uint16 and uint32 capture, selected-view/layered rendering and zero-base emulation. Full generated MSL is syntax-checked with `macos-metal2.4`; `static_assert` checks the reflected capture stride and every scalar offset against the Metal compiler's actual struct layout.

It also compiles the **actual emitted capture and replay address/mapping statements** as C++ under ASan/UBSan. Capture checks cover nonzero base instance, negative base vertex, repeated and wide indices, all legal combinations of first views `{0,4,30,31}` and view counts `{1,2,3,4,7,16,32}`, and three application instances. Replay checks cover reordered original corners, records shared between primitives, distinct view records, absolute primitive keys, nonzero occurrence/vertex/instance origins and the unchanged buffer ABI. This is CPU arithmetic and Metal syntax evidence, not GPU attribute-fetch or pipeline execution proof.

For an unchanged pre-fix checkout/library, run the same test driver with final argument `before`: it skips the unavailable new API and requires six specific real Metal syntax failures (layered widened uint8/uint16/uint32, ViewIndex-only, and direct/compute zero-base cases). Save its generated default shaders to compare against the fixed compiler. Use a separate checkout/library; never revert a shared tree for this comparison.
