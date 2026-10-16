# Builds a single acceptance-test plugin through the regular CMake build of the
# acceptance.sandbox target and copies the resulting artifact to OUTPUT.
#
# All scenarios share one sandbox source and one build tree, and build tools
# (Ninja, MSBuild, Make) do not support concurrent invocations on the same
# build tree. A cross-process lock serializes this script so scenarios can be
# executed in parallel by CTest.
#
# Required variables:
#   SOURCE          scenario specific plugin source
#   OUTPUT          destination path of the built plugin
#   SANDBOX_SOURCE  source file compiled by the acceptance.sandbox target
#   SANDBOX_TARGET  name of the sandbox target
#   ARTIFACT        path of the built sandbox target file
#   BUILD_DIR       top-level build directory
#   CONFIG          build configuration (may be empty)
#   LOCK_FILE       file used for inter-process locking

cmake_minimum_required(VERSION 3.24)

foreach(required SOURCE OUTPUT SANDBOX_SOURCE SANDBOX_TARGET ARTIFACT BUILD_DIR LOCK_FILE)
    if(NOT DEFINED ${required} OR "${${required}}" STREQUAL "")
        message(FATAL_ERROR "BuildSandbox.cmake: ${required} is not set")
    endif()
endforeach()

file(LOCK "${LOCK_FILE}" GUARD PROCESS TIMEOUT 1200 RESULT_VARIABLE lock_result)
if(NOT lock_result EQUAL 0)
    message(FATAL_ERROR "BuildSandbox.cmake: failed to acquire lock '${LOCK_FILE}': ${lock_result}")
endif()

# Read and write instead of copying, so the sandbox source always receives a
# fresh timestamp; preserving the original timestamp could make the build tool
# consider a stale object file up to date.
file(READ "${SOURCE}" source_content)
file(WRITE "${SANDBOX_SOURCE}" "${source_content}")

set(config_args "")
if(NOT "${CONFIG}" STREQUAL "")
    set(config_args --config "${CONFIG}")
endif()

execute_process(
    COMMAND "${CMAKE_COMMAND}" --build "${BUILD_DIR}" --target "${SANDBOX_TARGET}" ${config_args}
    RESULT_VARIABLE build_result
    OUTPUT_VARIABLE build_output
    ERROR_VARIABLE build_output
)

if(NOT build_result EQUAL 0)
    message(FATAL_ERROR "BuildSandbox.cmake: build of '${SANDBOX_TARGET}' failed (${build_result}):\n${build_output}")
endif()

file(COPY_FILE "${ARTIFACT}" "${OUTPUT}" RESULT copy_result)
if(NOT copy_result EQUAL 0)
    message(FATAL_ERROR "BuildSandbox.cmake: failed to copy '${ARTIFACT}' to '${OUTPUT}': ${copy_result}")
endif()

file(LOCK "${LOCK_FILE}" RELEASE)
