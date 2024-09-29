
add_library(cucumber-cpp-runner ${CCR_EXCLUDE_FROM_ALL})

target_sources(cucumber-cpp-runner PRIVATE
    Main.cpp
)

target_link_libraries(cucumber-cpp-runner PUBLIC
    cucumber-cpp
)
