function(target_apply_compiler_warnings target_name)
    if(MSVC)
        target_compile_options(${target_name} PRIVATE
            /W4
            /MP
            /utf-8
            /EHsc
            /permissive-
        )
    else()
        target_compile_options(${target_name} PRIVATE
            -Wall
            -Wextra
            -Wshadow
            -Wconversion
            -Wno-unused-parameter
        )
    endif()
endfunction()
