add_library(acceptance.steps OBJECT ${CCR_EXCLUDE_FROM_ALL})

target_sources(acceptance.steps PRIVATE
    Steps.cpp
)

target_link_libraries(acceptance.steps PRIVATE
    cucumber_cpp
    acceptance.util
)

target_compile_definitions(acceptance.steps PRIVATE
    CCR_ACCEPTANCE_CMAKE_COMMAND="${CMAKE_COMMAND}"
    CCR_ACCEPTANCE_BUILD_DIR="${CMAKE_BINARY_DIR}"
    CCR_ACCEPTANCE_BUILD_CONFIG="$<CONFIG>"
    CCR_ACCEPTANCE_SANDBOX_SOURCE="${CMAKE_CURRENT_BINARY_DIR}/../sandbox/Sandbox.cpp"
    CCR_ACCEPTANCE_SANDBOX_TARGET="acceptance.sandbox"
    CCR_ACCEPTANCE_SANDBOX_ARTIFACT="$<TARGET_FILE:acceptance.sandbox>"
    CCR_ACCEPTANCE_SANDBOX_LOCK="${CMAKE_CURRENT_BINARY_DIR}/../sandbox/sandbox.lock"
    CCR_ACCEPTANCE_SANDBOX_BUILD_SCRIPT="${PROJECT_SOURCE_DIR}/acceptance/sandbox/BuildSandbox.cmake"
    CCR_ACCEPTANCE_HOST_SOURCE="${CMAKE_CURRENT_BINARY_DIR}/../sandbox/SandboxHost.cpp"
    CCR_ACCEPTANCE_HOST_TARGET="acceptance.sandbox_host"
    CCR_ACCEPTANCE_HOST_ARTIFACT="$<TARGET_FILE:acceptance.sandbox_host>"
    CCR_ACCEPTANCE_HOST_SUFFIX="$<TARGET_FILE_SUFFIX:acceptance.sandbox_host>"
    CCR_ACCEPTANCE_HOST_RUNTIME_DLLS="$<JOIN:$<TARGET_RUNTIME_DLLS:acceptance.sandbox_host>,|>"
    CCR_ACCEPTANCE_CLI="$<TARGET_FILE:cucumber_cpp.cli>"
)
