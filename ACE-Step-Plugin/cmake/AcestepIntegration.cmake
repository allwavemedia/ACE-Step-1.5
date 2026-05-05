function(_acestep_apply_patch_if_needed source_dir patch_file)
    if(NOT EXISTS "${patch_file}")
        message(FATAL_ERROR "Required patch does not exist: ${patch_file}")
    endif()

    find_package(Git REQUIRED)

    execute_process(
        COMMAND "${GIT_EXECUTABLE}" -C "${source_dir}" apply --reverse --check "${patch_file}"
        RESULT_VARIABLE patch_reverse_result
        OUTPUT_QUIET
        ERROR_QUIET
    )

    if(patch_reverse_result EQUAL 0)
        message(STATUS "ACE-Step public headers patch is already applied.")
        return()
    endif()

    execute_process(
        COMMAND "${GIT_EXECUTABLE}" -C "${source_dir}" apply --check "${patch_file}"
        RESULT_VARIABLE patch_check_result
        OUTPUT_VARIABLE patch_check_output
        ERROR_VARIABLE patch_check_error
    )

    if(NOT patch_check_result EQUAL 0)
        message(STATUS
            "ACE-Step public headers patch does not apply cleanly; "
            "assuming upstream already changed."
        )
        message(STATUS "${patch_check_output}${patch_check_error}")
        return()
    endif()

    execute_process(
        COMMAND "${GIT_EXECUTABLE}" -C "${source_dir}" apply "${patch_file}"
        RESULT_VARIABLE patch_apply_result
        OUTPUT_VARIABLE patch_apply_output
        ERROR_VARIABLE patch_apply_error
    )

    if(NOT patch_apply_result EQUAL 0)
        message(FATAL_ERROR
            "Failed to apply ACE-Step patch: "
            "${patch_apply_output}${patch_apply_error}"
        )
    endif()
endfunction()

function(acestep_configure plugin_target)
    set(ACESTEP_CPP_DIR "${PROJECT_SOURCE_DIR}/External/acestep_cpp")

    if(NOT EXISTS "${ACESTEP_CPP_DIR}/CMakeLists.txt")
        message(FATAL_ERROR
            "acestep.cpp submodule is missing. "
            "Run: git submodule update --init --recursive"
        )
    endif()

    set(GGML_AVX2 ON CACHE BOOL "Build GGML AVX2 CPU kernels" FORCE)
    set(GGML_BACKEND_DL ON CACHE BOOL "Build GGML backends as runtime-loaded modules" FORCE)
    set(GGML_CPU ON CACHE BOOL "Build GGML CPU backend" FORCE)
    set(GGML_CUDA ON CACHE BOOL "Build GGML CUDA backend" FORCE)
    set(GGML_FMA ON CACHE BOOL "Build GGML FMA CPU kernels" FORCE)
    set(GGML_NATIVE OFF CACHE BOOL "Disable host-native CPU flags for DL backends" FORCE)
    set(GGML_OPENMP OFF CACHE BOOL "Disable OpenMP inside plugin hosts" FORCE)
    set(GGML_VULKAN ON CACHE BOOL "Build GGML Vulkan backend" FORCE)

    _acestep_apply_patch_if_needed(
        "${ACESTEP_CPP_DIR}"
        "${PROJECT_SOURCE_DIR}/patches/0001-public-headers.patch"
    )

    add_subdirectory(
        "${ACESTEP_CPP_DIR}"
        "${CMAKE_BINARY_DIR}/External/acestep_cpp"
        EXCLUDE_FROM_ALL
    )

    if(NOT TARGET acestep-core)
        message(FATAL_ERROR "Expected upstream target 'acestep-core' was not created.")
    endif()

    target_include_directories(${plugin_target}
        PRIVATE
            "${ACESTEP_CPP_DIR}"
            "${ACESTEP_CPP_DIR}/src"
    )

    target_link_libraries(${plugin_target} PRIVATE acestep-core)

    if(ACESTEP_PLUGIN_MODE STREQUAL "server")
        target_compile_definitions(${plugin_target} PRIVATE ACESTEP_PLUGIN_MODE_SERVER=1)
        if(TARGET ace-server AND TARGET ${plugin_target}_VST3)
            add_dependencies(${plugin_target}_VST3 ace-server)
        endif()
    else()
        target_compile_definitions(${plugin_target} PRIVATE ACESTEP_PLUGIN_MODE_STATIC=1)
    endif()
endfunction()
