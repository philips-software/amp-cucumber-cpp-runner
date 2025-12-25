add_library(cucumber_cpp.library.util STATIC ${CCR_EXCLUDE_FROM_ALL})

target_sources(cucumber_cpp.library.util PRIVATE
    Broadcaster.cpp
    Broadcaster.hpp
    GetWorstTestStepResult.hpp
    GetWorstTestStepResult.cpp
    Immoveable.hpp
)

target_include_directories(cucumber_cpp.library.util PUBLIC
    ../../..
)

target_link_libraries(cucumber_cpp.library.util PUBLIC
    cucumber_gherkin_lib
)
