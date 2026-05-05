function(acestep_bundle_backends vst3_target)
    if(NOT TARGET ${vst3_target})
        message(FATAL_ERROR "Cannot bundle GGML backends; target does not exist: ${vst3_target}")
    endif()

    add_custom_command(TARGET ${vst3_target} POST_BUILD
        COMMAND "${CMAKE_COMMAND}"
            -DACESTEP_BUILD_DIR="$<SHELL_PATH:${CMAKE_BINARY_DIR}>"
            -DPLUGIN_BINARY_DIR="$<SHELL_PATH:$<TARGET_FILE_DIR:${vst3_target}>>"
            -P "${CMAKE_CURRENT_FUNCTION_LIST_DIR}/CopyBackends.cmake"
        COMMENT "Copying GGML backend DLLs into the VST3 bundle"
        VERBATIM
    )

    if(ACESTEP_PLUGIN_MODE STREQUAL "server" AND TARGET ace-server)
        add_custom_command(TARGET ${vst3_target} POST_BUILD
            COMMAND "${CMAKE_COMMAND}" -E copy_if_different
                "$<TARGET_FILE:ace-server>"
                "$<TARGET_FILE_DIR:${vst3_target}>/"
            COMMENT "Copying ace-server sidecar into the VST3 bundle"
            VERBATIM
        )
    endif()
endfunction()
