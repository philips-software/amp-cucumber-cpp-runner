set(CMAKE_COMPILE_WARNING_AS_ERROR On)

add_library(cucumber_cpp.acceptance_test.util OBJECT ${CCR_EXCLUDE_FROM_ALL})

target_sources(cucumber_cpp.acceptance_test.util PRIVATE
    NdjsonComparer.cpp
    NdjsonComparer.h
)

target_link_libraries(cucumber_cpp.acceptance_test.util PUBLIC
    nlohmann_json
)
