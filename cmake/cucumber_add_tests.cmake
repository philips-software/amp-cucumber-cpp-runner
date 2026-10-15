# Distributed under the Apache-2.0 License.
# Helper script to discover and register cucumber-cpp scenarios in CTest.

cmake_minimum_required(VERSION 3.24)

if(POLICY CMP0174)
  cmake_policy(SET CMP0174 NEW)
endif()

function(cucumber_discover_tests_impl)
  set(options "")
  set(oneValueArgs
    TEST_TARGET
    TEST_EXECUTABLE
    TEST_WORKING_DIR
    TEST_PREFIX
    TEST_SUFFIX
    TEST_LIST
    CTEST_FILE
    TEST_DISCOVERY_TIMEOUT
    TEST_EXTRA_ARGS
    TEST_PROPERTIES
    TEST_EXECUTOR
  )
  set(multiValueArgs "")
  cmake_parse_arguments(PARSE_ARGV 0 arg
    "${options}" "${oneValueArgs}" "${multiValueArgs}"
  )

  set(prefix "${arg_TEST_PREFIX}")
  set(suffix "${arg_TEST_SUFFIX}")
  set(script "")
  set(tests "")

  file(REMOVE "${arg_CTEST_FILE}")

  if(NOT EXISTS "${arg_TEST_EXECUTABLE}")
    message(FATAL_ERROR
      "Specified test executable does not exist.\n"
      "  Path: '${arg_TEST_EXECUTABLE}'"
    )
  endif()

  # Build extra arguments
  set(extra_args_list "")
  if(NOT "${arg_TEST_EXTRA_ARGS}" STREQUAL "")
    set(extra_args_list ${arg_TEST_EXTRA_ARGS})
  endif()

  set(extra_args_script "")
  foreach(extra_arg IN LISTS extra_args_list)
    string(APPEND extra_args_script " [==[${extra_arg}]==]")
  endforeach()

  set(launcher_args "")
  if(NOT "${arg_TEST_EXECUTOR}" STREQUAL "")
    set(launcher_args ${arg_TEST_EXECUTOR})
  endif()

  # Run test executable with --list-scenarios to get available scenarios
  execute_process(
    COMMAND ${launcher_args} "${arg_TEST_EXECUTABLE}" ${extra_args_list} --list-scenarios
    WORKING_DIRECTORY "${arg_TEST_WORKING_DIR}"
    TIMEOUT "${arg_TEST_DISCOVERY_TIMEOUT}"
    OUTPUT_VARIABLE output
    ERROR_VARIABLE error_output
    RESULT_VARIABLE result
  )

  if(NOT result EQUAL 0)
    message(FATAL_ERROR
      "Error running cucumber test executable to discover scenarios.\n"
      "  Path: '${arg_TEST_EXECUTABLE}'\n"
      "  Working directory: '${arg_TEST_WORKING_DIR}'\n"
      "  Result: ${result}\n"
      "  Output: ${output}\n"
      "  Error: ${error_output}\n"
    )
  endif()

  # Split output into lines
  string(REPLACE "\r\n" "\n" output "${output}")
  string(REPLACE "\r" "\n" output "${output}")
  string(REPLACE ";" "\\;" output "${output}")
  string(REPLACE "\n" ";" output "${output}")

  foreach(line IN LISTS output)
    string(STRIP "${line}" line)
    if(line STREQUAL "")
      continue()
    endif()

    if(line MATCHES "^([^\t]+)\t(.*)$")
      set(test_name "${CMAKE_MATCH_1}")
      set(source_location "${CMAKE_MATCH_2}")
    else()
      set(test_name "${line}")
      set(source_location "")
    endif()

    # Escape regex special characters in test_name for exact matching
    string(REGEX REPLACE "([\\^\\$\\.\\*\\+\\?\\(\\)\\[\\]\\{\\}\\|\\\\])" "\\\\\\1" escaped_test_name "${test_name}")

    set(full_test_name "${prefix}${test_name}${suffix}")

    string(APPEND script "add_test([==[${full_test_name}]==] [==[${arg_TEST_EXECUTABLE}]==]${extra_args_script} [==[--name]==] [==[^${escaped_test_name}$]==])\n")
    string(APPEND script "set_tests_properties([==[${full_test_name}]==] PROPERTIES WORKING_DIRECTORY [==[${arg_TEST_WORKING_DIR}]==])\n")

    if(NOT "${source_location}" STREQUAL "")
      string(APPEND script "set_tests_properties([==[${full_test_name}]==] PROPERTIES DEF_SOURCE_LINE [==[${source_location}]==])\n")
    endif()

    if(NOT "${arg_TEST_PROPERTIES}" STREQUAL "")
      string(APPEND script "set_tests_properties([==[${full_test_name}]==] PROPERTIES ${arg_TEST_PROPERTIES})\n")
    endif()

    list(APPEND tests "${full_test_name}")
  endforeach()

  if(NOT "${arg_TEST_LIST}" STREQUAL "")
    string(APPEND script "set([==[${arg_TEST_LIST}]==] [==[${tests}]==])\n")
  endif()

  file(WRITE "${arg_CTEST_FILE}" "${script}")
endfunction()

if(CMAKE_SCRIPT_MODE_FILE)
  cucumber_discover_tests_impl(
    TEST_TARGET "${TEST_TARGET}"
    TEST_EXECUTABLE "${TEST_EXECUTABLE}"
    TEST_EXECUTOR "${TEST_EXECUTOR}"
    TEST_WORKING_DIR "${TEST_WORKING_DIR}"
    TEST_PREFIX "${TEST_PREFIX}"
    TEST_SUFFIX "${TEST_SUFFIX}"
    TEST_LIST "${TEST_LIST}"
    CTEST_FILE "${CTEST_FILE}"
    TEST_DISCOVERY_TIMEOUT "${TEST_DISCOVERY_TIMEOUT}"
    TEST_EXTRA_ARGS "${TEST_EXTRA_ARGS}"
    TEST_PROPERTIES "${TEST_PROPERTIES}"
  )
endif()
