add_library(cucumber_cpp.runner STATIC ${CCR_EXCLUDE_FROM_ALL})

target_sources(cucumber_cpp.runner PRIVATE
    Main.cpp
)

target_link_libraries(cucumber_cpp.runner PUBLIC
    cucumber_cpp
)

add_executable(cucumber_cpp.cli ${CCR_EXCLUDE_FROM_ALL})

target_link_libraries(cucumber_cpp.cli PRIVATE
    cucumber_cpp.runner
)

set_target_properties(cucumber_cpp.cli PROPERTIES
    ENABLE_EXPORTS ON
)

if (NOT WIN32)
    target_link_libraries(cucumber_cpp.cli PRIVATE
        $<LINK_LIBRARY:WHOLE_ARCHIVE,cucumber_cpp.library,cucumber_cpp.library.api,cucumber_cpp.library.engine,cucumber_cpp.library.support,cucumber_cpp.library.util,GTest::gtest,GTest::gmock>
    )
endif()

# Note: executables that link cucumber_cpp.runner and use --load for plugins
# must set ENABLE_EXPORTS ON to export symbols for dynamic library resolution:
#   set_target_properties(my_executable PROPERTIES ENABLE_EXPORTS ON)
