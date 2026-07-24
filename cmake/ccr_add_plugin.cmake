# Helper function for creating cucumber-cpp-runner plugin targets
#
# Usage:
#   ccr_add_plugin(my_plugin)
#   ccr_add_plugin(my_plugin step1.cpp step2.cpp hooks.cpp)
#
# Creates a MODULE library target suitable for dynamic loading via --load.
# Plugins use the same STEP/HOOK/PARAMETER macros as statically-linked code.
# All symbols resolve from the host executable at runtime (ENABLE_EXPORTS).
# On macOS, -undefined dynamic_lookup suppresses linker errors for host symbols.
# On Windows, the plugin links against the host executable's import library.
#
# Sources can be passed directly (like add_library) or added later via
# target_sources.

function(ccr_add_plugin TARGET_NAME)
    add_library(${TARGET_NAME} MODULE ${ARGN})

    # Plugins need the header include paths from cucumber_cpp and its
    # transitive dependencies (fmt, gtest, gherkin, etc.) but must NOT link
    # the actual libraries — symbols resolve from the host at runtime.
    target_include_directories(${TARGET_NAME} PRIVATE
        $<TARGET_PROPERTY:cucumber_cpp,INTERFACE_INCLUDE_DIRECTORIES>
        $<TARGET_PROPERTY:cucumber_cpp.library,INCLUDE_DIRECTORIES>
    )

    target_compile_definitions(${TARGET_NAME} PRIVATE
        $<TARGET_PROPERTY:cucumber_cpp.library,COMPILE_DEFINITIONS>
    )

    if (WIN32)
        target_link_libraries(${TARGET_NAME} PRIVATE cucumber_cpp)
    elseif (APPLE)
        target_link_options(${TARGET_NAME} PRIVATE -undefined dynamic_lookup)
    endif()

    set_target_properties(${TARGET_NAME} PROPERTIES
        PREFIX ""
        CXX_VISIBILITY_PRESET default
    )
endfunction()
