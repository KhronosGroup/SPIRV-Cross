; SPIR-V
; Version: 1.0
; Generator: Wine VKD3D Shader Compiler; 0
; Bound: 367
; Schema: 0
               OpCapability Shader
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %main "main" %o0
               OpName %main "main"
               OpName %t0 "t0"
               OpName %o0 "o0"
               OpName %r0 "r0"
               OpName %push_cb "push_cb"
               OpMemberName %push_cb 0 "cb0"
               OpName %switch0_merge "switch0_merge"
               OpName %switch0_case4294967295 "switch0_case4294967295"
               OpName %switch1_merge "switch1_merge"
               OpName %switch1_case4294967294 "switch1_case4294967294"
               OpName %dummy_sampler "dummy_sampler"
               OpName %switch1_case4294967295 "switch1_case4294967295"
               OpName %switch1_case0 "switch1_case0"
               OpName %switch1_case1 "switch1_case1"
               OpName %switch1_case2 "switch1_case2"
               OpName %switch1_default "switch1_default"
               OpName %switch0_case0 "switch0_case0"
               OpName %switch2_merge "switch2_merge"
               OpName %switch2_case4294967294 "switch2_case4294967294"
               OpName %switch2_case4294967295 "switch2_case4294967295"
               OpName %switch2_case0 "switch2_case0"
               OpName %switch2_case1 "switch2_case1"
               OpName %switch2_case2 "switch2_case2"
               OpName %switch2_default "switch2_default"
               OpName %switch0_case1 "switch0_case1"
               OpName %switch3_merge "switch3_merge"
               OpName %switch3_case4294967294 "switch3_case4294967294"
               OpName %switch3_case4294967295 "switch3_case4294967295"
               OpName %switch3_case0 "switch3_case0"
               OpName %switch3_case1 "switch3_case1"
               OpName %switch3_case2 "switch3_case2"
               OpName %switch3_default "switch3_default"
               OpName %switch0_default "switch0_default"
               OpDecorate %t0 DescriptorSet 0
               OpDecorate %t0 Binding 2
               OpDecorate %o0 Location 0
               OpDecorate %_arr_v4float_uint_1 ArrayStride 16
               OpDecorate %push_cb Block
               OpMemberDecorate %push_cb 0 Offset 0
               OpDecorate %dummy_sampler DescriptorSet 0
               OpDecorate %dummy_sampler Binding 4
       %void = OpTypeVoid
          %2 = OpTypeFunction %void
      %float = OpTypeFloat 32
          %6 = OpTypeImage %float 2D 0 0 0 1 Unknown
%_ptr_UniformConstant_6 = OpTypePointer UniformConstant %6
         %t0 = OpVariable %_ptr_UniformConstant_6 UniformConstant
    %v4float = OpTypeVector %float 4
%_ptr_Output_v4float = OpTypePointer Output %v4float
         %o0 = OpVariable %_ptr_Output_v4float Output
%_ptr_Function_v4float = OpTypePointer Function %v4float
       %uint = OpTypeInt 32 0
     %uint_1 = OpConstant %uint 1
%_arr_v4float_uint_1 = OpTypeArray %v4float %uint_1
    %push_cb = OpTypeStruct %_arr_v4float_uint_1
%_ptr_PushConstant_push_cb = OpTypePointer PushConstant %push_cb
         %19 = OpVariable %_ptr_PushConstant_push_cb PushConstant
     %uint_0 = OpConstant %uint 0
%_ptr_PushConstant_v4float = OpTypePointer PushConstant %v4float
%_ptr_PushConstant_float = OpTypePointer PushConstant %float
        %int = OpTypeInt 32 1
    %v2float = OpTypeVector %float 2
    %float_0 = OpConstant %float 0
         %42 = OpConstantComposite %v2float %float_0 %float_0
         %45 = OpTypeSampler
%_ptr_UniformConstant_45 = OpTypePointer UniformConstant %45
%dummy_sampler = OpVariable %_ptr_UniformConstant_45 UniformConstant
         %50 = OpTypeSampledImage %6
      %v2int = OpTypeVector %int 2
%_ptr_Function_float = OpTypePointer Function %float
     %uint_3 = OpConstant %uint 3
     %int_n1 = OpConstant %int -1
     %int_n2 = OpConstant %int -2
         %64 = OpConstantComposite %v2int %int_n1 %int_n2
         %83 = OpConstantComposite %v2int %int_n1 %int_n1
      %int_0 = OpConstant %int 0
        %103 = OpConstantComposite %v2int %int_n1 %int_0
      %int_1 = OpConstant %int 1
        %123 = OpConstantComposite %v2int %int_n1 %int_1
      %int_2 = OpConstant %int 2
        %143 = OpConstantComposite %v2int %int_n1 %int_2
        %169 = OpConstantComposite %v2int %int_0 %int_n2
        %188 = OpConstantComposite %v2int %int_0 %int_n1
        %225 = OpConstantComposite %v2int %int_0 %int_1
        %244 = OpConstantComposite %v2int %int_0 %int_2
        %270 = OpConstantComposite %v2int %int_1 %int_n2
        %289 = OpConstantComposite %v2int %int_1 %int_n1
        %308 = OpConstantComposite %v2int %int_1 %int_0
        %327 = OpConstantComposite %v2int %int_1 %int_1
        %346 = OpConstantComposite %v2int %int_1 %int_2
       %main = OpFunction %void None %2
          %4 = OpLabel
         %r0 = OpVariable %_ptr_Function_v4float Function
         %23 = OpAccessChain %_ptr_PushConstant_v4float %19 %uint_0 %uint_0
         %25 = OpInBoundsAccessChain %_ptr_PushConstant_float %23 %uint_0
         %26 = OpLoad %float %25
         %28 = OpBitcast %int %26
               OpSelectionMerge %switch0_merge None
               OpSwitch %28 %switch0_default -1 %switch0_case4294967295 0 %switch0_case0 1 %switch0_case1
%switch0_case4294967295 = OpLabel
         %31 = OpAccessChain %_ptr_PushConstant_v4float %19 %uint_0 %uint_0
         %32 = OpInBoundsAccessChain %_ptr_PushConstant_float %31 %uint_1
         %33 = OpLoad %float %32
         %34 = OpBitcast %int %33
               OpSelectionMerge %switch1_merge None
               OpSwitch %34 %switch1_default -2 %switch1_case4294967294 -1 %switch1_case4294967295 0 %switch1_case0 1 %switch1_case1 2 %switch1_case2
%switch1_case4294967294 = OpLabel
         %36 = OpAccessChain %_ptr_PushConstant_v4float %19 %uint_0 %uint_0
         %37 = OpLoad %v4float %36
         %38 = OpLoad %v4float %r0
         %39 = OpVectorShuffle %v4float %38 %37 6 7 2 3
               OpStore %r0 %39
         %43 = OpLoad %v4float %r0
         %44 = OpVectorShuffle %v4float %43 %42 0 1 4 5
               OpStore %r0 %44
         %48 = OpLoad %6 %t0
         %49 = OpLoad %45 %dummy_sampler
         %51 = OpSampledImage %50 %48 %49
         %52 = OpImage %6 %51
         %53 = OpLoad %v4float %r0
         %54 = OpVectorShuffle %v2float %53 %53 0 1
         %56 = OpBitcast %v2int %54
         %59 = OpInBoundsAccessChain %_ptr_Function_float %r0 %uint_3
         %60 = OpLoad %float %59
         %61 = OpBitcast %int %60
         %65 = OpImageFetch %v4float %52 %56 Lod|ConstOffset %61 %64
               OpStore %o0 %65
               OpReturn
%switch1_case4294967295 = OpLabel
         %67 = OpAccessChain %_ptr_PushConstant_v4float %19 %uint_0 %uint_0
         %68 = OpLoad %v4float %67
         %69 = OpLoad %v4float %r0
         %70 = OpVectorShuffle %v4float %69 %68 6 7 2 3
               OpStore %r0 %70
         %71 = OpLoad %v4float %r0
         %72 = OpVectorShuffle %v4float %71 %42 0 1 4 5
               OpStore %r0 %72
         %73 = OpLoad %6 %t0
         %74 = OpLoad %45 %dummy_sampler
         %75 = OpSampledImage %50 %73 %74
         %76 = OpImage %6 %75
         %77 = OpLoad %v4float %r0
         %78 = OpVectorShuffle %v2float %77 %77 0 1
         %79 = OpBitcast %v2int %78
         %80 = OpInBoundsAccessChain %_ptr_Function_float %r0 %uint_3
         %81 = OpLoad %float %80
         %82 = OpBitcast %int %81
         %84 = OpImageFetch %v4float %76 %79 Lod|ConstOffset %82 %83
               OpStore %o0 %84
               OpReturn
%switch1_case0 = OpLabel
         %86 = OpAccessChain %_ptr_PushConstant_v4float %19 %uint_0 %uint_0
         %87 = OpLoad %v4float %86
         %88 = OpLoad %v4float %r0
         %89 = OpVectorShuffle %v4float %88 %87 6 7 2 3
               OpStore %r0 %89
         %90 = OpLoad %v4float %r0
         %91 = OpVectorShuffle %v4float %90 %42 0 1 4 5
               OpStore %r0 %91
         %92 = OpLoad %6 %t0
         %93 = OpLoad %45 %dummy_sampler
         %94 = OpSampledImage %50 %92 %93
         %95 = OpImage %6 %94
         %96 = OpLoad %v4float %r0
         %97 = OpVectorShuffle %v2float %96 %96 0 1
         %98 = OpBitcast %v2int %97
         %99 = OpInBoundsAccessChain %_ptr_Function_float %r0 %uint_3
        %100 = OpLoad %float %99
        %101 = OpBitcast %int %100
        %104 = OpImageFetch %v4float %95 %98 Lod|ConstOffset %101 %103
               OpStore %o0 %104
               OpReturn
%switch1_case1 = OpLabel
        %106 = OpAccessChain %_ptr_PushConstant_v4float %19 %uint_0 %uint_0
        %107 = OpLoad %v4float %106
        %108 = OpLoad %v4float %r0
        %109 = OpVectorShuffle %v4float %108 %107 6 7 2 3
               OpStore %r0 %109
        %110 = OpLoad %v4float %r0
        %111 = OpVectorShuffle %v4float %110 %42 0 1 4 5
               OpStore %r0 %111
        %112 = OpLoad %6 %t0
        %113 = OpLoad %45 %dummy_sampler
        %114 = OpSampledImage %50 %112 %113
        %115 = OpImage %6 %114
        %116 = OpLoad %v4float %r0
        %117 = OpVectorShuffle %v2float %116 %116 0 1
        %118 = OpBitcast %v2int %117
        %119 = OpInBoundsAccessChain %_ptr_Function_float %r0 %uint_3
        %120 = OpLoad %float %119
        %121 = OpBitcast %int %120
        %124 = OpImageFetch %v4float %115 %118 Lod|ConstOffset %121 %123
               OpStore %o0 %124
               OpReturn
%switch1_case2 = OpLabel
        %126 = OpAccessChain %_ptr_PushConstant_v4float %19 %uint_0 %uint_0
        %127 = OpLoad %v4float %126
        %128 = OpLoad %v4float %r0
        %129 = OpVectorShuffle %v4float %128 %127 6 7 2 3
               OpStore %r0 %129
        %130 = OpLoad %v4float %r0
        %131 = OpVectorShuffle %v4float %130 %42 0 1 4 5
               OpStore %r0 %131
        %132 = OpLoad %6 %t0
        %133 = OpLoad %45 %dummy_sampler
        %134 = OpSampledImage %50 %132 %133
        %135 = OpImage %6 %134
        %136 = OpLoad %v4float %r0
        %137 = OpVectorShuffle %v2float %136 %136 0 1
        %138 = OpBitcast %v2int %137
        %139 = OpInBoundsAccessChain %_ptr_Function_float %r0 %uint_3
        %140 = OpLoad %float %139
        %141 = OpBitcast %int %140
        %144 = OpImageFetch %v4float %135 %138 Lod|ConstOffset %141 %143
               OpStore %o0 %144
               OpReturn
%switch1_default = OpLabel
               OpBranch %switch1_merge
%switch1_merge = OpLabel
               OpBranch %switch0_merge
%switch0_case0 = OpLabel
        %148 = OpAccessChain %_ptr_PushConstant_v4float %19 %uint_0 %uint_0
        %149 = OpInBoundsAccessChain %_ptr_PushConstant_float %148 %uint_1
        %150 = OpLoad %float %149
        %151 = OpBitcast %int %150
               OpSelectionMerge %switch2_merge None
               OpSwitch %151 %switch2_default -2 %switch2_case4294967294 -1 %switch2_case4294967295 0 %switch2_case0 1 %switch2_case1 2 %switch2_case2
%switch2_case4294967294 = OpLabel
        %153 = OpAccessChain %_ptr_PushConstant_v4float %19 %uint_0 %uint_0
        %154 = OpLoad %v4float %153
        %155 = OpLoad %v4float %r0
        %156 = OpVectorShuffle %v4float %155 %154 6 7 2 3
               OpStore %r0 %156
        %157 = OpLoad %v4float %r0
        %158 = OpVectorShuffle %v4float %157 %42 0 1 4 5
               OpStore %r0 %158
        %159 = OpLoad %6 %t0
        %160 = OpLoad %45 %dummy_sampler
        %161 = OpSampledImage %50 %159 %160
        %162 = OpImage %6 %161
        %163 = OpLoad %v4float %r0
        %164 = OpVectorShuffle %v2float %163 %163 0 1
        %165 = OpBitcast %v2int %164
        %166 = OpInBoundsAccessChain %_ptr_Function_float %r0 %uint_3
        %167 = OpLoad %float %166
        %168 = OpBitcast %int %167
        %170 = OpImageFetch %v4float %162 %165 Lod|ConstOffset %168 %169
               OpStore %o0 %170
               OpReturn
%switch2_case4294967295 = OpLabel
        %172 = OpAccessChain %_ptr_PushConstant_v4float %19 %uint_0 %uint_0
        %173 = OpLoad %v4float %172
        %174 = OpLoad %v4float %r0
        %175 = OpVectorShuffle %v4float %174 %173 6 7 2 3
               OpStore %r0 %175
        %176 = OpLoad %v4float %r0
        %177 = OpVectorShuffle %v4float %176 %42 0 1 4 5
               OpStore %r0 %177
        %178 = OpLoad %6 %t0
        %179 = OpLoad %45 %dummy_sampler
        %180 = OpSampledImage %50 %178 %179
        %181 = OpImage %6 %180
        %182 = OpLoad %v4float %r0
        %183 = OpVectorShuffle %v2float %182 %182 0 1
        %184 = OpBitcast %v2int %183
        %185 = OpInBoundsAccessChain %_ptr_Function_float %r0 %uint_3
        %186 = OpLoad %float %185
        %187 = OpBitcast %int %186
        %189 = OpImageFetch %v4float %181 %184 Lod|ConstOffset %187 %188
               OpStore %o0 %189
               OpReturn
%switch2_case0 = OpLabel
        %191 = OpAccessChain %_ptr_PushConstant_v4float %19 %uint_0 %uint_0
        %192 = OpLoad %v4float %191
        %193 = OpLoad %v4float %r0
        %194 = OpVectorShuffle %v4float %193 %192 6 7 2 3
               OpStore %r0 %194
        %195 = OpLoad %v4float %r0
        %196 = OpVectorShuffle %v4float %195 %42 0 1 4 5
               OpStore %r0 %196
        %197 = OpLoad %6 %t0
        %198 = OpLoad %45 %dummy_sampler
        %199 = OpSampledImage %50 %197 %198
        %200 = OpImage %6 %199
        %201 = OpLoad %v4float %r0
        %202 = OpVectorShuffle %v2float %201 %201 0 1
        %203 = OpBitcast %v2int %202
        %204 = OpInBoundsAccessChain %_ptr_Function_float %r0 %uint_3
        %205 = OpLoad %float %204
        %206 = OpBitcast %int %205
        %207 = OpImageFetch %v4float %200 %203 Lod %206
               OpStore %o0 %207
               OpReturn
%switch2_case1 = OpLabel
        %209 = OpAccessChain %_ptr_PushConstant_v4float %19 %uint_0 %uint_0
        %210 = OpLoad %v4float %209
        %211 = OpLoad %v4float %r0
        %212 = OpVectorShuffle %v4float %211 %210 6 7 2 3
               OpStore %r0 %212
        %213 = OpLoad %v4float %r0
        %214 = OpVectorShuffle %v4float %213 %42 0 1 4 5
               OpStore %r0 %214
        %215 = OpLoad %6 %t0
        %216 = OpLoad %45 %dummy_sampler
        %217 = OpSampledImage %50 %215 %216
        %218 = OpImage %6 %217
        %219 = OpLoad %v4float %r0
        %220 = OpVectorShuffle %v2float %219 %219 0 1
        %221 = OpBitcast %v2int %220
        %222 = OpInBoundsAccessChain %_ptr_Function_float %r0 %uint_3
        %223 = OpLoad %float %222
        %224 = OpBitcast %int %223
        %226 = OpImageFetch %v4float %218 %221 Lod|ConstOffset %224 %225
               OpStore %o0 %226
               OpReturn
%switch2_case2 = OpLabel
        %228 = OpAccessChain %_ptr_PushConstant_v4float %19 %uint_0 %uint_0
        %229 = OpLoad %v4float %228
        %230 = OpLoad %v4float %r0
        %231 = OpVectorShuffle %v4float %230 %229 6 7 2 3
               OpStore %r0 %231
        %232 = OpLoad %v4float %r0
        %233 = OpVectorShuffle %v4float %232 %42 0 1 4 5
               OpStore %r0 %233
        %234 = OpLoad %6 %t0
        %235 = OpLoad %45 %dummy_sampler
        %236 = OpSampledImage %50 %234 %235
        %237 = OpImage %6 %236
        %238 = OpLoad %v4float %r0
        %239 = OpVectorShuffle %v2float %238 %238 0 1
        %240 = OpBitcast %v2int %239
        %241 = OpInBoundsAccessChain %_ptr_Function_float %r0 %uint_3
        %242 = OpLoad %float %241
        %243 = OpBitcast %int %242
        %245 = OpImageFetch %v4float %237 %240 Lod|ConstOffset %243 %244
               OpStore %o0 %245
               OpReturn
%switch2_default = OpLabel
               OpBranch %switch2_merge
%switch2_merge = OpLabel
               OpBranch %switch0_merge
%switch0_case1 = OpLabel
        %249 = OpAccessChain %_ptr_PushConstant_v4float %19 %uint_0 %uint_0
        %250 = OpInBoundsAccessChain %_ptr_PushConstant_float %249 %uint_1
        %251 = OpLoad %float %250
        %252 = OpBitcast %int %251
               OpSelectionMerge %switch3_merge None
               OpSwitch %252 %switch3_default -2 %switch3_case4294967294 -1 %switch3_case4294967295 0 %switch3_case0 1 %switch3_case1 2 %switch3_case2
%switch3_case4294967294 = OpLabel
        %254 = OpAccessChain %_ptr_PushConstant_v4float %19 %uint_0 %uint_0
        %255 = OpLoad %v4float %254
        %256 = OpLoad %v4float %r0
        %257 = OpVectorShuffle %v4float %256 %255 6 7 2 3
               OpStore %r0 %257
        %258 = OpLoad %v4float %r0
        %259 = OpVectorShuffle %v4float %258 %42 0 1 4 5
               OpStore %r0 %259
        %260 = OpLoad %6 %t0
        %261 = OpLoad %45 %dummy_sampler
        %262 = OpSampledImage %50 %260 %261
        %263 = OpImage %6 %262
        %264 = OpLoad %v4float %r0
        %265 = OpVectorShuffle %v2float %264 %264 0 1
        %266 = OpBitcast %v2int %265
        %267 = OpInBoundsAccessChain %_ptr_Function_float %r0 %uint_3
        %268 = OpLoad %float %267
        %269 = OpBitcast %int %268
        %271 = OpImageFetch %v4float %263 %266 Lod|ConstOffset %269 %270
               OpStore %o0 %271
               OpReturn
%switch3_case4294967295 = OpLabel
        %273 = OpAccessChain %_ptr_PushConstant_v4float %19 %uint_0 %uint_0
        %274 = OpLoad %v4float %273
        %275 = OpLoad %v4float %r0
        %276 = OpVectorShuffle %v4float %275 %274 6 7 2 3
               OpStore %r0 %276
        %277 = OpLoad %v4float %r0
        %278 = OpVectorShuffle %v4float %277 %42 0 1 4 5
               OpStore %r0 %278
        %279 = OpLoad %6 %t0
        %280 = OpLoad %45 %dummy_sampler
        %281 = OpSampledImage %50 %279 %280
        %282 = OpImage %6 %281
        %283 = OpLoad %v4float %r0
        %284 = OpVectorShuffle %v2float %283 %283 0 1
        %285 = OpBitcast %v2int %284
        %286 = OpInBoundsAccessChain %_ptr_Function_float %r0 %uint_3
        %287 = OpLoad %float %286
        %288 = OpBitcast %int %287
        %290 = OpImageFetch %v4float %282 %285 Lod|ConstOffset %288 %289
               OpStore %o0 %290
               OpReturn
%switch3_case0 = OpLabel
        %292 = OpAccessChain %_ptr_PushConstant_v4float %19 %uint_0 %uint_0
        %293 = OpLoad %v4float %292
        %294 = OpLoad %v4float %r0
        %295 = OpVectorShuffle %v4float %294 %293 6 7 2 3
               OpStore %r0 %295
        %296 = OpLoad %v4float %r0
        %297 = OpVectorShuffle %v4float %296 %42 0 1 4 5
               OpStore %r0 %297
        %298 = OpLoad %6 %t0
        %299 = OpLoad %45 %dummy_sampler
        %300 = OpSampledImage %50 %298 %299
        %301 = OpImage %6 %300
        %302 = OpLoad %v4float %r0
        %303 = OpVectorShuffle %v2float %302 %302 0 1
        %304 = OpBitcast %v2int %303
        %305 = OpInBoundsAccessChain %_ptr_Function_float %r0 %uint_3
        %306 = OpLoad %float %305
        %307 = OpBitcast %int %306
        %309 = OpImageFetch %v4float %301 %304 Lod|ConstOffset %307 %308
               OpStore %o0 %309
               OpReturn
%switch3_case1 = OpLabel
        %311 = OpAccessChain %_ptr_PushConstant_v4float %19 %uint_0 %uint_0
        %312 = OpLoad %v4float %311
        %313 = OpLoad %v4float %r0
        %314 = OpVectorShuffle %v4float %313 %312 6 7 2 3
               OpStore %r0 %314
        %315 = OpLoad %v4float %r0
        %316 = OpVectorShuffle %v4float %315 %42 0 1 4 5
               OpStore %r0 %316
        %317 = OpLoad %6 %t0
        %318 = OpLoad %45 %dummy_sampler
        %319 = OpSampledImage %50 %317 %318
        %320 = OpImage %6 %319
        %321 = OpLoad %v4float %r0
        %322 = OpVectorShuffle %v2float %321 %321 0 1
        %323 = OpBitcast %v2int %322
        %324 = OpInBoundsAccessChain %_ptr_Function_float %r0 %uint_3
        %325 = OpLoad %float %324
        %326 = OpBitcast %int %325
        %328 = OpImageFetch %v4float %320 %323 Lod|ConstOffset %326 %327
               OpStore %o0 %328
               OpReturn
%switch3_case2 = OpLabel
        %330 = OpAccessChain %_ptr_PushConstant_v4float %19 %uint_0 %uint_0
        %331 = OpLoad %v4float %330
        %332 = OpLoad %v4float %r0
        %333 = OpVectorShuffle %v4float %332 %331 6 7 2 3
               OpStore %r0 %333
        %334 = OpLoad %v4float %r0
        %335 = OpVectorShuffle %v4float %334 %42 0 1 4 5
               OpStore %r0 %335
        %336 = OpLoad %6 %t0
        %337 = OpLoad %45 %dummy_sampler
        %338 = OpSampledImage %50 %336 %337
        %339 = OpImage %6 %338
        %340 = OpLoad %v4float %r0
        %341 = OpVectorShuffle %v2float %340 %340 0 1
        %342 = OpBitcast %v2int %341
        %343 = OpInBoundsAccessChain %_ptr_Function_float %r0 %uint_3
        %344 = OpLoad %float %343
        %345 = OpBitcast %int %344
        %347 = OpImageFetch %v4float %339 %342 Lod|ConstOffset %345 %346
               OpStore %o0 %347
               OpReturn
%switch3_default = OpLabel
               OpBranch %switch3_merge
%switch3_merge = OpLabel
               OpBranch %switch0_merge
%switch0_default = OpLabel
               OpBranch %switch0_merge
%switch0_merge = OpLabel
        %350 = OpAccessChain %_ptr_PushConstant_v4float %19 %uint_0 %uint_0
        %351 = OpLoad %v4float %350
        %352 = OpLoad %v4float %r0
        %353 = OpVectorShuffle %v4float %352 %351 6 7 2 3
               OpStore %r0 %353
        %354 = OpLoad %v4float %r0
        %355 = OpVectorShuffle %v4float %354 %42 0 1 4 5
               OpStore %r0 %355
        %356 = OpLoad %6 %t0
        %357 = OpLoad %45 %dummy_sampler
        %358 = OpSampledImage %50 %356 %357
        %359 = OpImage %6 %358
        %360 = OpLoad %v4float %r0
        %361 = OpVectorShuffle %v2float %360 %360 0 1
        %362 = OpBitcast %v2int %361
        %363 = OpInBoundsAccessChain %_ptr_Function_float %r0 %uint_3
        %364 = OpLoad %float %363
        %365 = OpBitcast %int %364
        %366 = OpImageFetch %v4float %359 %362 Lod %365
               OpStore %o0 %366
               OpReturn
               OpFunctionEnd
