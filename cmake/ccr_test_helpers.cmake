function(ccr_enable_testing)
    include(GoogleTest)

    if (CCR_ENABLE_COVERAGE)
        add_compile_options(
            -g -O0 --coverage -fprofile-arcs -ftest-coverage -fno-inline
            $<$<COMPILE_LANGUAGE:CXX>:-fno-elide-constructors>
        )

        if (CMAKE_C_COMPILER_ID STREQUAL "GNU")
            link_libraries(gcov)
        else()
            add_link_options(--coverage)
        endif()
    endif()
endfunction()
