# Portable fragment barycentrics

Explicit C++ opt-in for `BuiltInBaryCoordKHR` and `BuiltInBaryCoordNoPerspKHR`, including their NV aliases. Both can be active together. This generates MSL 2.4 user varyings and never requires Metal `barycentric_coord`. Without the setter, the native backend, restrictions and emitted attributes are unchanged. No C API, CLI option, extension advertisement, MoltenVK change or GPU-support claim is included.

```cpp
CompilerMSL compiler(spirv);
auto options = compiler.get_msl_options();
options.set_msl_version(2, 4);
compiler.set_msl_options(options);
MSLFragmentBarycentricInputBinding barycentrics;
barycentrics.perspective_location = 12;
barycentrics.no_perspective_location = 14;
compiler.set_msl_fragment_barycentric_input(barycentrics);
std::string msl = compiler.compile();
```

Configure before compilation. Only active builtins require a location; the other may remain `~0u`. When both are active, reserve distinct locations. Private locations must not collide with active application inputs (including matrix columns, Component inputs and captured PerVertex inputs), any non-builtin `add_msl_shader_input` mapping at that location (a builtin mapping occupies no user Location), or the active `MSLPerVertexInputBinding::primitive_index_location`. Locations are not auto-allocated or exposed as application inputs by `is_msl_shader_input_used()`. Original SPIR-V BuiltIn reflection is retained. Include the opt-in and both locations in shader/pipeline cache identity.

## Producer contract

The rasterizing producer supplies a **float3** at each configured active location. Each triangle corner writes the following values to **both** varyings:

| Corner in the original PerVertexKHR order | Perspective location | No-perspective location |
| --- | --- | --- |
| 0 | `(1, 0, 0)` | `(1, 0, 0)` |
| 1 | `(0, 1, 0)` | `(0, 1, 0)` |
| 2 | `(0, 0, 1)` | `(0, 0, 1)` |

For the example configuration, the producer fields are `float3 ... [[user(locn12)]]` and `float3 ... [[user(locn14)]]`. The fragment selects perspective versus no-perspective interpolation. Supply the same unscaled one-hot basis at both locations: do not premultiply by clip `w`, divide by `w`, or normalize it yourself. The producer must preserve the application's clip-space Position, especially its `w`.

At the selected evaluation position, let `lambda[j]` be screen-space triangle weights and `w[j]` the corners' clip-space `w`. Interpolating the basis yields:

```text
BaryCoordKHR[j]        = (lambda[j] / w[j]) / sum_k(lambda[k] / w[k])
BaryCoordNoPerspKHR[j] = lambda[j]
```

`Centroid` and `Sample` select the evaluation position independently for each builtin. Without either, interpolation is at the center. NoPersp uses no-perspective interpolation even without a `NoPerspective` decoration. A `NoPerspective` decoration on the perspective builtin is rejected, matching the existing native diagnostic.

For captured PerVertex consumption, corner `j` must correspond exactly to `primitive_indices[3 * uint64(privatePrimitiveKey) + j]`. All corners share the primitive key, but each corner has its own basis. A shared indexed vertex may require a separate replay occurrence for each primitive/corner. The caller owns triangle assembly, restart, clipping/rasterization compatibility, degenerates, corner ordering, instance/view selection and pipeline linkage. This contract is for triangles; point/line barycentrics are not inferred.

The two-argument `compile_captured_output_replay()` retains its private flat uint key, captured application outputs and `(record, primitive_key)` occurrence pairs. The optional three-argument overload now supplies this basis from an explicit corner buffer; see [replay barycentric producer](msl_captured_output_replay.md#optional-triangle-barycentric-producer) for its binding, occurrence indexing, validation and shared-record example. It never infers a corner from `vertex_id % 3`. The fragment opt-in is independent of the replay producer and `set_msl_per_vertex_input_buffer()` and does not enable either the portable or native PerVertex path automatically.

## Explicit interpolation and failures

Direct builtin variables and BuiltIn-decorated block members support ordinary loads, including helper functions. `GLSL.std.450 InterpolateAtCentroid`, `InterpolateAtSample` and `InterpolateAtOffset` use the existing MSL `interpolant<float3, interpolation::perspective/no_perspective>` path. The ordinary loads still use the builtin's default center/centroid/sample setting. Whole vectors, scalar access chains, runtime vector components and helper interpolation are covered. The existing offset conversion (`offset + 0.4375`) is retained.

Whole-variable `InterpolateAt*` uses activate the portable builtins even when there is no `OpLoad`, including in helpers and without any ordinary user input. Binding validation therefore also applies to explicit-only shaders.

In a BuiltIn-decorated input block containing either barycentric declaration, active `FragCoord`, `FrontFacing`, `PointCoord`, `SampleId`, `SamplePosition`, `HelperInvocation`, `SampleMask`, `ClipDistance`, `CullDistance`, `PrimitiveId`, `Layer` and `ViewportIndex` members use their ordinary builtin lowering. Companion access chains are lowered to standalone builtin variables before opcode preprocessing and helper argument extraction. They do not inherit the barycentric block's pull-model interpolation. Clip/cull arrays use `user(clipN)` / `user(cullN)` inputs; SampleMask uses Metal's scalar `uint`; HelperInvocation uses either `simd_is_helper_thread()` or shared mutable helper state according to the existing option. FragCoord and SamplePosition retain their existing fixups.

The mixed-block fixture covers ordinary loads, helpers, whole-array SampleMask and ClipDistance loads, runtime clip indices, CullDistance, integer companions, demotion followed by helper reads, and both helper-update modes. It also covers inactive barycentric members, inactive unsupported companions, and SampleMask filtering with both a fixed mask and sample-rate shading. Whole-array SampleMask loads reconstruct the one-word SPIR-V array; helper arguments pass the filtered scalar by value.

`OpCopyObject` chains now preserve the backing Input variable, interpolation interface index, scalar component and lvalue representation. Copied block roots are resolved before builtin activity analysis. The copied-pointer fixture covers direct variables, BuiltIn block members, copies before and after access chains, repeated copies, constant/runtime vector components, ordinary varying interpolation, constant array elements, helper-only explicit interpolation and ordinary copied scalar loads. An empty access chain after a scalar component selection retains that component through a subsequent copy. Centroid, sample and offset operations retain their requested operation and perspective mode. Source assertions check the actual barycentric member names and scalar component, preventing a syntactically valid interpolation of a different value.

The following layouts still fail closed:

- Other active companion BuiltIns in a barycentric block: `MSL fragment input blocks with builtin members do not support active builtin <N>.` A valid `ViewIndex` fixture checks value `4440`; the supported list above is exhaustive.
- SampleMask arrays other than one literal word: `MSL fragment input blocks with builtin members require a single SampleMask word.` A validated two-word fixture exercises this boundary, even with inactive barycentrics. Specialization-dependent lengths are also rejected.
- Active SampleMask members in a barycentric block with `force_native_arrays = true`: `MSL fragment input blocks with builtin members do not support SampleMask with force_native_arrays.` Whole-array SampleMask loads require a representation unavailable in this mode. Inactive SampleMask members and standalone SampleMask inputs retain their existing behavior.
- Whole BuiltIn block loads or arrayed block access: `MSL fragment input blocks with builtin members require direct member access.` A valid whole-block load through a copied pointer exercises this boundary.
- Dynamic indexing of arrays under pull-model interpolation retains the existing diagnostic: `Trying to dynamically index into an array interface variable using pull-model interpolation. This is currently unsupported.` Runtime **vector components** are supported.
- Nested pull-model composites (multidimensional arrays, arrays of matrices/structs, nested structs): `Portable barycentric explicit interpolation does not support nested input composites.` A validated copied two-dimensional array fixture checks this; their flattened offsets require composite strides, so they must not be treated as a sum of scalar member indices.
- Pointers which still cannot be resolved during preprocessing: `Portable barycentric inputs do not support explicit interpolation through an unresolved input pointer.` A missing interface-member mapping at emission is also rejected: `Portable barycentric explicit interpolation requires a resolved input member.` Neither case substitutes default interpolation or member zero.

### Validity and lowering

The fixtures are validated with `spirv-val --target-env vulkan1.1`. `GLSL.std.450 InterpolateAtCentroid/Sample/Offset` requires an Input pointer to a matching 32-bit float scalar/vector; the operand need not be a direct variable or an access chain. `OpCopyObject` of an Input pointer preserves its pointee and storage class. SPIRV-Tools' `validate_extensions.cpp` and `validate_memory.cpp` enforce these rules. Vulkan BuiltIn interface structs require Block, all their members must be BuiltIn-decorated, and a struct containing BuiltIn members cannot be nested inside another struct (`validate_decorations.cpp`, `validate_type.cpp`). Per-builtin type/stage restrictions still apply (`validate_builtins.cpp`): float3 fragment barycentrics, bool HelperInvocation, integer-array SampleMask and float-array clip/cull distances. These tests do not use illegal mixtures of Location and BuiltIn members to demonstrate support.

References: [GLSL.std.450 specification](https://registry.khronos.org/SPIR-V/specs/unified1/GLSL.std.450.html), [SPIR-V specification](https://registry.khronos.org/SPIR-V/specs/unified1/SPIRV.html), [Vulkan shader interfaces](https://registry.khronos.org/vulkan/specs/latest/html/vkspec.html#interfaces).

Other checked failures include non-fragment entry points, MSL below 2.4, missing/duplicate private locations, collisions, non-float3 barycentrics, Flat/Patch/PerVertexKHR or simultaneous Centroid/Sample decorations, and reconfiguration after compilation has started. The shader must otherwise be valid SPIR-V and satisfy the existing backend's requirements.

## Reproduction and evidence

The CMake test target assembles/compiles and validates the ten source fixtures as Vulkan 1.1 SPIR-V in the build directory. No checked-in binary regeneration is needed.

```sh
cmake -S . -B build/barycentric-portable/compiler -G Ninja -DCMAKE_BUILD_TYPE=Release -DSPIRV_CROSS_ENABLE_TESTS=ON -Dspirv-cross-glslang=$(command -v glslangValidator) -Dspirv-cross-spirv-as=$(command -v spirv-as) -Dspirv-cross-spirv-val=$(command -v spirv-val) -Dspirv-cross-spirv-opt=$(command -v spirv-opt)
cmake --build build/barycentric-portable/compiler -j 6
ctest --test-dir build/barycentric-portable/compiler -R 'spirv-cross-msl-(barycentric|capture|per-vertex|interface-locations)' --output-on-failure
bash tests-other/msl_barycentric_portable.sh build/barycentric-portable/compiler build/barycentric-portable/metal
```

The shell check performs CPU code generation and `xcrun metal -std=macos-metal2.4 -fsyntax-only -fno-modules` only. It never creates a GPU device, pipeline or command buffer. The C++ test covers both builtins, all nine independent interpolation-decoration combinations with and without explicit interpolation, single-builtin private mappings, builtin blocks, explicit NoPerspective, builtin/application reflection, invalid configurations, captured PerVertex coexistence, the one-hot perspective equation with nonuniform `w`, and native attribute/rejection regressions.

Run the existing native MSL reference corpora using:

```sh
python3 test_shaders.py shaders-msl --msl --force-no-external-validation --parallel --spirv-cross build/barycentric-portable/compiler/spirv-cross --glslang $(command -v glslangValidator) --spirv-as $(command -v spirv-as) --spirv-val $(command -v spirv-val) --spirv-opt $(command -v spirv-opt)
```

Repeat with `--opt` for optimized references, then replace `shaders-msl` with `shaders-msl-no-opt` without `--opt`. These check unchanged native output against the existing corpus; the new C++ fixtures exercise the opt-in, which has no CLI switch. Metal syntax success and the CPU basis calculation do not establish GPU interpolation accuracy, pipeline support, replay integration or rendering on the host in U state.
