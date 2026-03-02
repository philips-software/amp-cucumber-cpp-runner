add_library(cucumber_cpp.library.util ${CCR_EXCLUDE_FROM_ALL})

target_sources(cucumber_cpp.library.util PRIVATE
    ArgumentGroupToMessageGroup.cpp
    ArgumentGroupToMessageGroup.hpp
    Broadcaster.cpp
    Broadcaster.hpp
    Duration.cpp
    Duration.hpp
    GetWorstTestStepResult.cpp
    GetWorstTestStepResult.hpp
    Immoveable.hpp
    Timestamp.cpp
    Timestamp.hpp
    TransformDocString.cpp
    TransformDocString.hpp
    TransformTable.cpp
    TransformTable.hpp
    Trim.hpp
)

target_include_directories(cucumber_cpp.library.util PUBLIC
    ../../..
)

target_link_libraries(cucumber_cpp.library.util PUBLIC
    cucumber_gherkin_lib
    fmt::fmt
)

if (CCR_BUILD_TESTS)
    add_subdirectory(test)
    # add_subdirectory(test_helper)
endif()
