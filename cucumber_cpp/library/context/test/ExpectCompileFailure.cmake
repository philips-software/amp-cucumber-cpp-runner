set(build_command "${CMAKE_COMMAND}" --build "${BUILD_DIRECTORY}" --target "${TEST_TARGET}")
if(NOT BUILD_CONFIG STREQUAL "")
    list(APPEND build_command --config "${BUILD_CONFIG}")
endif()

execute_process(
    COMMAND ${build_command}
    RESULT_VARIABLE result
    OUTPUT_VARIABLE output
    ERROR_VARIABLE error
)

if(result STREQUAL "0")
    message(FATAL_ERROR "${TEST_TARGET} unexpectedly compiled")
endif()

string(FIND "${output}\n${error}" "${EXPECTED_DIAGNOSTIC}" diagnostic_position)
if(diagnostic_position EQUAL -1)
    message(FATAL_ERROR "Compilation failed without the expected diagnostic:\n${output}\n${error}")
endif()
