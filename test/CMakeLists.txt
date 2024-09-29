set(CMAKE_COMPILE_WARNING_AS_ERROR On)

add_executable(cucumber-cpp-test ${CCR_EXCLUDE_FROM_ALL})

target_sources(cucumber-cpp-test PRIVATE
    Main.cpp
)
target_link_libraries(cucumber-cpp-test PRIVATE
    cucumber-cpp
    cucumber-cpp-test.hooks
    cucumber-cpp-test.steps
)

add_executable(cucumber-cpp-test-custom ${CCR_EXCLUDE_FROM_ALL})

target_sources(cucumber-cpp-test-custom PRIVATE
    MainCustom.cpp
)

target_link_libraries(cucumber-cpp-test-custom PRIVATE
    cucumber-cpp
    cucumber-cpp-test.hooks
    cucumber-cpp-test.steps
)

add_subdirectory(hooks)
add_subdirectory(steps)
