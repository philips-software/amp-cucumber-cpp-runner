add_executable(cucumber_cpp.library.context.benchmark)

target_link_libraries(cucumber_cpp.library.context.benchmark PUBLIC
    benchmark::benchmark_main
    cucumber_cpp.library.context
)

target_sources(cucumber_cpp.library.context.benchmark PRIVATE
    BmContext.cpp
)
