file(GLOB ACESTEP_BACKEND_DLLS
    "${ACESTEP_BUILD_DIR}/ggml-*.dll"
    "${ACESTEP_BUILD_DIR}/External/acestep_cpp/ggml-*.dll"
)

if(NOT ACESTEP_BACKEND_DLLS)
    message(WARNING "No ggml-*.dll backends found under ${ACESTEP_BUILD_DIR}.")
    return()
endif()

foreach(backend_dll IN LISTS ACESTEP_BACKEND_DLLS)
    file(COPY "${backend_dll}" DESTINATION "${PLUGIN_BINARY_DIR}")
endforeach()
