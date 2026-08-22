add_library(cucumber_cpp.library.plugin ${CCR_EXCLUDE_FROM_ALL})

target_sources(cucumber_cpp.library.plugin PRIVATE
    DynamicLibrary.cpp
    DynamicLibrary.hpp
    DynamicLibraryManager.cpp
    DynamicLibraryManager.hpp
    HookLoader.cpp
    HookLoader.hpp
    ParameterLoader.cpp
    ParameterLoader.hpp
    PluginExport.hpp
    StepLoader.cpp
    StepLoader.hpp
)

target_include_directories(cucumber_cpp.library.plugin PUBLIC
    ../../..
)

target_link_libraries(cucumber_cpp.library.plugin PUBLIC
    cucumber_cpp.library.support
    cucumber_cpp.library.cucumber_expression
    ${CMAKE_DL_LIBS}
)

if (CCR_BUILD_TESTS)
    add_subdirectory(test)
endif()
