function(__get_predefined_config_params args)
    set(${args}
        -DBUILD_SHARED_LIBS=OFF
        -DLLVM_BUILD_EXAMPLES=OFF
        -DLLVM_BUILD_TESTS=OFF
        -DLLVM_BUILD_DOCS=OFF
        -DSPIRV_BUILD_TESTS=OFF
        -DLLVM_ENABLE_WARNINGS=OFF
        -DLLVM_ENABLE_WERROR=OFF
        -DINSTALL_GTEST=OFF
        -DBUILD_GMOCK=OFF
        -Dgtest_force_shared_crt=ON
        -DSPIRV_SKIP_EXECUTABLES=ON
        -DENABLE_SPIRV_CODEGEN=ON
        -DCLANG_INCLUDE_TESTS=OFF
        -DLLVM_INCLUDE_TESTS=OFF
        -DHLSL_INCLUDE_TESTS=OFF
        -DHLSL_BUILD_DXILCONV=OFF
        -DHLSL_SUPPORT_QUERY_GIT_COMMIT_INFO=ON
        -DLLVM_INCLUDE_DOCS=OFF
        -DLLVM_INCLUDE_EXAMPLES=OFF
        -DLIBCLANG_BUILD_STATIC=ON
        -DLLVM_OPTIMIZED_TABLEGEN=OFF
        -DLLVM_REQUIRES_EH=ON
        -DLLVM_APPEND_VC_REV=ON
        -DLLVM_ENABLE_RTTI=ON
        -DLLVM_ENABLE_EH=ON
        -DLLVM_ENABLE_TERMINFO=OFF
        -DCLANG_BUILD_EXAMPLES=OFF
        -DLLVM_REQUIRES_RTTI=ON
        -DSPIRV_BUILD_TESTS=OFF
        -DSPIRV_SKIP_TESTS=ON
        -DCLANG_ENABLE_STATIC_ANALYZER=OFF
        -DCLANG_ENABLE_ARCMT=OFF
        -DCLANG_CL=OFF
        -DLLVM_TARGETS_TO_BUILD=None
        -DLLVM_DEFAULT_TARGET_TRIPLE=dxil-ms-dx
        PARENT_SCOPE
    )
endfunction()
