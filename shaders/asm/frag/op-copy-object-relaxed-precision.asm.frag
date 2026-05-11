; Verifies that OpCopyObject's result precision is determined by its OWN
; RelaxedPrecision decoration, not by the operand's. The SPIR-V spec defines
; OpCopyObject as: "Make a copy of Operand. There are no pointer dereferences
; involved." Result decorations are independent of the operand's, which makes
; OpCopyObject a natural way to express explicit precision conversion (e.g.
; inserting a non-RP shadow of an RP value, or vice versa). SPIR-V-Cross's
; forward_relaxed_precision() incorrectly propagates the operand's RP onto
; the result, which silently changes the consumer's evaluation precision.
;
; Test layout:
;   %src is mediump (RelaxedPrecision-decorated input)
;   %copy_mp = OpCopyObject %src        decorated RelaxedPrecision -> mediump
;   %copy_hp = OpCopyObject %src        NOT RelaxedPrecision         -> highp
;
;   dst_mp gets copy_mp directly.
;   dst_hp gets `copy_hp << 16` — the shift must be evaluated at highp
;   precision; mediump uint is only guaranteed 16-bit, so a shift by 16
;   is implementation-defined and produces 0 (or otherwise wrong) on
;   conforming drivers.
;
; Expected GLSL output:
;   mediump uint copy_mp = src;
;   uint copy_hp = src;                  (not mediump — operates as highp)
;   dst_hp = copy_hp << 16u;
;
; Without the fix, spirv-cross propagates %src's RelaxedPrecision through
; OpCopyObject onto %copy_hp, emitting it as `mediump uint`. The shift
; then operates at mediump precision and produces wrong device output:
; with mediump uint at 16-bit, `value << 16` is undefined/zero, instead
; of the intended `src << 16` highp value.
;
; SPIR-V
; Version: 1.0
; Generator: Khronos Glslang Reference Front End; 11
; Bound: 22
; Schema: 0
               OpCapability Shader
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %main "main" %src %dst_mp %dst_hp
               OpExecutionMode %main OriginUpperLeft
               OpSource ESSL 310
               OpName %main "main"
               OpName %src "src"
               OpName %dst_mp "dst_mp"
               OpName %dst_hp "dst_hp"
               OpName %copy_mp "copy_mp"
               OpName %copy_hp "copy_hp"
               OpDecorate %src Flat
               OpDecorate %src Location 0
               OpDecorate %src RelaxedPrecision
               OpDecorate %dst_mp Location 0
               OpDecorate %dst_mp RelaxedPrecision
               OpDecorate %dst_hp Location 1
               OpDecorate %copy_mp RelaxedPrecision
       %void = OpTypeVoid
          %3 = OpTypeFunction %void
       %uint = OpTypeInt 32 0
%_ptr_Input_uint = OpTypePointer Input %uint
%_ptr_Output_uint = OpTypePointer Output %uint
        %src = OpVariable %_ptr_Input_uint Input
     %dst_mp = OpVariable %_ptr_Output_uint Output
     %dst_hp = OpVariable %_ptr_Output_uint Output
    %uint_16 = OpConstant %uint 16
       %main = OpFunction %void None %3
          %5 = OpLabel
         %10 = OpLoad %uint %src
    %copy_mp = OpCopyObject %uint %10
    %copy_hp = OpCopyObject %uint %10
      %shift = OpShiftLeftLogical %uint %copy_hp %uint_16
               OpStore %dst_mp %copy_mp
               OpStore %dst_hp %shift
               OpReturn
               OpFunctionEnd
