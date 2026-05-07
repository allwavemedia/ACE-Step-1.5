file(TO_CMAKE_PATH "${ACESTEP_BUILD_DIR}" ACESTEP_BACKEND_BUILD_DIR)
file(TO_CMAKE_PATH "${PLUGIN_BINARY_DIR}" ACESTEP_PLUGIN_BINARY_DIR)

file(GLOB ACESTEP_BACKEND_DLLS
    "${ACESTEP_BACKEND_BUILD_DIR}/ggml.dll"
    "${ACESTEP_BACKEND_BUILD_DIR}/ggml-*.dll"
    "${ACESTEP_BACKEND_BUILD_DIR}/External/acestep_cpp/ggml.dll"
    "${ACESTEP_BACKEND_BUILD_DIR}/External/acestep_cpp/ggml-*.dll"
)

if(NOT ACESTEP_BACKEND_DLLS)
    message(WARNING "No ggml backend DLLs found under ${ACESTEP_BACKEND_BUILD_DIR}.")
    return()
endif()

foreach(backend_dll IN LISTS ACESTEP_BACKEND_DLLS)
    file(COPY "${backend_dll}" DESTINATION "${ACESTEP_PLUGIN_BINARY_DIR}")
endforeach()
