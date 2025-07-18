               OpCapability Shader
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %_entrypoint_v "_entrypoint" %sk_FragColor
               OpExecutionMode %_entrypoint_v OriginUpperLeft

               ; Debug Information
               OpName %sk_FragColor "sk_FragColor"  ; id %3
               OpName %_UniformBuffer "_UniformBuffer"  ; id %8
               OpMemberName %_UniformBuffer 0 "testInputs"
               OpMemberName %_UniformBuffer 1 "colorGreen"
               OpMemberName %_UniformBuffer 2 "colorRed"
               OpName %_entrypoint_v "_entrypoint_v"    ; id %10
               OpName %main "main"                      ; id %2
               OpName %expected "expected"              ; id %23

               ; Annotations
               OpDecorate %main RelaxedPrecision
               OpDecorate %sk_FragColor RelaxedPrecision
               OpDecorate %sk_FragColor Location 0
               OpDecorate %sk_FragColor Index 0
               OpMemberDecorate %_UniformBuffer 0 Offset 0
               OpMemberDecorate %_UniformBuffer 0 RelaxedPrecision
               OpMemberDecorate %_UniformBuffer 1 Offset 16
               OpMemberDecorate %_UniformBuffer 1 RelaxedPrecision
               OpMemberDecorate %_UniformBuffer 2 Offset 32
               OpMemberDecorate %_UniformBuffer 2 RelaxedPrecision
               OpDecorate %_UniformBuffer Block
               OpDecorate %7 Binding 0
               OpDecorate %7 DescriptorSet 0
               OpDecorate %expected RelaxedPrecision
               OpDecorate %28 RelaxedPrecision
               OpDecorate %33 RelaxedPrecision
               OpDecorate %34 RelaxedPrecision
               OpDecorate %38 RelaxedPrecision
               OpDecorate %40 RelaxedPrecision
               OpDecorate %41 RelaxedPrecision
               OpDecorate %42 RelaxedPrecision
               OpDecorate %49 RelaxedPrecision
               OpDecorate %51 RelaxedPrecision
               OpDecorate %52 RelaxedPrecision
               OpDecorate %54 RelaxedPrecision
               OpDecorate %61 RelaxedPrecision
               OpDecorate %63 RelaxedPrecision
               OpDecorate %103 RelaxedPrecision
               OpDecorate %106 RelaxedPrecision
               OpDecorate %107 RelaxedPrecision

               ; Types, variables and constants
      %float = OpTypeFloat 32
    %v4float = OpTypeVector %float 4
%_ptr_Output_v4float = OpTypePointer Output %v4float
%sk_FragColor = OpVariable %_ptr_Output_v4float Output  ; RelaxedPrecision, Location 0, Index 0
%_UniformBuffer = OpTypeStruct %v4float %v4float %v4float   ; Block
%_ptr_Uniform__UniformBuffer = OpTypePointer Uniform %_UniformBuffer
          %7 = OpVariable %_ptr_Uniform__UniformBuffer Uniform  ; Binding 0, DescriptorSet 0
       %void = OpTypeVoid
         %12 = OpTypeFunction %void
    %float_0 = OpConstant %float 0
    %v2float = OpTypeVector %float 2
         %16 = OpConstantComposite %v2float %float_0 %float_0
%_ptr_Function_v2float = OpTypePointer Function %v2float
         %20 = OpTypeFunction %v4float %_ptr_Function_v2float
%_ptr_Function_v4float = OpTypePointer Function %v4float
         %25 = OpConstantComposite %v4float %float_0 %float_0 %float_0 %float_0
       %bool = OpTypeBool
      %false = OpConstantFalse %bool
%_ptr_Uniform_v4float = OpTypePointer Uniform %v4float
        %int = OpTypeInt 32 1
      %int_0 = OpConstant %int 0
     %v2bool = OpTypeVector %bool 2
    %v3float = OpTypeVector %float 3
     %v3bool = OpTypeVector %bool 3
     %v4bool = OpTypeVector %bool 4
    %float_1 = OpConstant %float 1
         %84 = OpConstantComposite %v2float %float_1 %float_1
         %93 = OpConstantComposite %v2float %float_0 %float_1
      %int_1 = OpConstant %int 1
      %int_2 = OpConstant %int 2


               ; Function _entrypoint_v
%_entrypoint_v = OpFunction %void None %12

         %13 = OpLabel
         %17 =   OpVariable %_ptr_Function_v2float Function
                 OpStore %17 %16
         %19 =   OpFunctionCall %v4float %main %17
                 OpStore %sk_FragColor %19
                 OpReturn
               OpFunctionEnd


               ; Function main
       %main = OpFunction %v4float None %20         ; RelaxedPrecision
         %21 = OpFunctionParameter %_ptr_Function_v2float

         %22 = OpLabel
   %expected =   OpVariable %_ptr_Function_v4float Function     ; RelaxedPrecision
         %97 =   OpVariable %_ptr_Function_v4float Function
                 OpStore %expected %25
         %29 =   OpAccessChain %_ptr_Uniform_v4float %7 %int_0
         %33 =   OpLoad %v4float %29                ; RelaxedPrecision
         %34 =   OpCompositeExtract %float %33 0    ; RelaxedPrecision
         %28 =   OpDPdy %float %34                  ; RelaxedPrecision
         %35 =   OpFOrdEqual %bool %28 %float_0
                 OpSelectionMerge %37 None
                 OpBranchConditional %35 %36 %37

         %36 =     OpLabel
         %39 =       OpAccessChain %_ptr_Uniform_v4float %7 %int_0
         %40 =       OpLoad %v4float %39            ; RelaxedPrecision
         %41 =       OpVectorShuffle %v2float %40 %40 0 1   ; RelaxedPrecision
         %38 =       OpDPdy %v2float %41                    ; RelaxedPrecision
         %42 =       OpVectorShuffle %v2float %25 %25 0 1   ; RelaxedPrecision
         %43 =       OpFOrdEqual %v2bool %38 %42
         %45 =       OpAll %bool %43
                     OpBranch %37

         %37 = OpLabel
         %46 =   OpPhi %bool %false %22 %45 %36
                 OpSelectionMerge %48 None
                 OpBranchConditional %46 %47 %48

         %47 =     OpLabel
         %50 =       OpAccessChain %_ptr_Uniform_v4float %7 %int_0
         %51 =       OpLoad %v4float %50            ; RelaxedPrecision
         %52 =       OpVectorShuffle %v3float %51 %51 0 1 2     ; RelaxedPrecision
         %49 =       OpDPdy %v3float %52                        ; RelaxedPrecision
         %54 =       OpVectorShuffle %v3float %25 %25 0 1 2     ; RelaxedPrecision
         %55 =       OpFOrdEqual %v3bool %49 %54
         %57 =       OpAll %bool %55
                     OpBranch %48

         %48 = OpLabel
         %58 =   OpPhi %bool %false %37 %57 %47
                 OpSelectionMerge %60 None
                 OpBranchConditional %58 %59 %60

         %59 =     OpLabel
         %62 =       OpAccessChain %_ptr_Uniform_v4float %7 %int_0
         %63 =       OpLoad %v4float %62            ; RelaxedPrecision
         %61 =       OpDPdy %v4float %63            ; RelaxedPrecision
         %64 =       OpFOrdEqual %v4bool %61 %25
         %66 =       OpAll %bool %64
                     OpBranch %60

         %60 = OpLabel
         %67 =   OpPhi %bool %false %48 %66 %59
                 OpSelectionMerge %69 None
                 OpBranchConditional %67 %68 %69

         %68 =     OpLabel
         %72 =       OpLoad %v2float %21
         %73 =       OpVectorShuffle %v2float %72 %72 0 0
         %71 =       OpDPdy %v2float %73
         %70 =       OpExtInst %v2float %1 FSign %71
         %74 =       OpFOrdEqual %v2bool %70 %16
         %75 =       OpAll %bool %74
                     OpBranch %69

         %69 = OpLabel
         %76 =   OpPhi %bool %false %60 %75 %68
                 OpSelectionMerge %78 None
                 OpBranchConditional %76 %77 %78

         %77 =     OpLabel
         %81 =       OpLoad %v2float %21
         %82 =       OpVectorShuffle %v2float %81 %81 1 1
         %80 =       OpDPdy %v2float %82
         %79 =       OpExtInst %v2float %1 FSign %80
         %85 =       OpFOrdEqual %v2bool %79 %84
         %86 =       OpAll %bool %85
                     OpBranch %78

         %78 = OpLabel
         %87 =   OpPhi %bool %false %69 %86 %77
                 OpSelectionMerge %89 None
                 OpBranchConditional %87 %88 %89

         %88 =     OpLabel
         %92 =       OpLoad %v2float %21
         %91 =       OpDPdy %v2float %92
         %90 =       OpExtInst %v2float %1 FSign %91
         %94 =       OpFOrdEqual %v2bool %90 %93
         %95 =       OpAll %bool %94
                     OpBranch %89

         %89 = OpLabel
         %96 =   OpPhi %bool %false %78 %95 %88
                 OpSelectionMerge %100 None
                 OpBranchConditional %96 %98 %99

         %98 =     OpLabel
        %101 =       OpAccessChain %_ptr_Uniform_v4float %7 %int_1
        %103 =       OpLoad %v4float %101           ; RelaxedPrecision
                     OpStore %97 %103
                     OpBranch %100

         %99 =     OpLabel
        %104 =       OpAccessChain %_ptr_Uniform_v4float %7 %int_2
        %106 =       OpLoad %v4float %104           ; RelaxedPrecision
                     OpStore %97 %106
                     OpBranch %100

        %100 = OpLabel
        %107 =   OpLoad %v4float %97                ; RelaxedPrecision
                 OpReturnValue %107
               OpFunctionEnd
