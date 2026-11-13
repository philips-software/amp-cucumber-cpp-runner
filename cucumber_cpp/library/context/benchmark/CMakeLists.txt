add_executable(cucumber_cpp.library.benchmark)

target_link_libraries(cucumber_cpp.library.benchmark PUBLIC
    benchmark::benchmark_main
    cucumber_cpp.library
)

target_sources(cucumber_cpp.library.benchmark PRIVATE
    BmContext.cpp
)
