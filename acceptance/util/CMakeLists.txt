add_library(acceptance.util ${CCR_EXCLUDE_FROM_ALL})

target_sources(acceptance.util PRIVATE
    Subprocess.cpp
    Subprocess.hpp
)

target_include_directories(acceptance.util PUBLIC
    ${PROJECT_SOURCE_DIR}
)
