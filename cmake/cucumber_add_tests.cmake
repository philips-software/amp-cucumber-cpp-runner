# Distributed under the Apache-2.0 License.
# Helper script to discover and register cucumber-cpp scenarios in CTest.

# Scope policy settings to this file so including it never alters the
# caller's policies; functions defined here record these settings.
cmake_policy(PUSH)
cmake_policy(VERSION 3.24)

if(POLICY CMP0174)
  cmake_policy(SET CMP0174 NEW)
endif()

# Quote VALUE as a CMake bracket argument, choosing a bracket level that does
# not clash with the content, and store the result in OUT_VAR.
function(_cucumber_bracket_argument out_var value)
  set(level "==")
  string(LENGTH "${value}" length)
  while(TRUE)
    string(FIND "${value}]${level}]" "]${level}]" position)
    if(position EQUAL length)
      break()
    endif()
    string(APPEND level "=")
  endwhile()
  # A newline directly after the opening bracket is ignored by the parser
  if(value MATCHES "^[\r\n]")
    set(value "\n${value}")
  endif()
  set(${out_var} "[${level}[${value}]${level}]" PARENT_SCOPE)
endfunction()

# Quote every element of the list VALUES as a separate bracket argument and
# store the space-separated result in OUT_VAR.
function(_cucumber_bracket_arguments out_var values)
  set(result "")
  foreach(value IN LISTS values)
    _cucumber_bracket_argument(quoted "${value}")
    string(APPEND result " ${quoted}")
  endforeach()
  set(${out_var} "${result}" PARENT_SCOPE)
endfunction()

# Escape all ECMAScript regex metacharacters in VALUE and store the result in
# OUT_VAR. The backslash is escaped first so added escapes are not doubled.
function(_cucumber_escape_regex out_var value)
  string(REPLACE "\\" "\\\\" value "${value}")
  foreach(char IN ITEMS "^" "$" "." "*" "+" "?" "(" ")" "[" "]" "{" "}" "|")
    string(REPLACE "${char}" "\\${char}" value "${value}")
  endforeach()
  set(${out_var} "${value}" PARENT_SCOPE)
endfunction()

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

  _cucumber_bracket_arguments(extra_args_script "${extra_args_list}")
  _cucumber_bracket_argument(quoted_executable "${arg_TEST_EXECUTABLE}")
  _cucumber_bracket_argument(quoted_working_dir "${arg_TEST_WORKING_DIR}")
  _cucumber_bracket_arguments(properties_script "${arg_TEST_PROPERTIES}")

  set(launcher_args "")
  if(NOT "${arg_TEST_EXECUTOR}" STREQUAL "")
    set(launcher_args ${arg_TEST_EXECUTOR})
  endif()

  _cucumber_bracket_arguments(launcher_args_script "${launcher_args}")

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

  # Split output into lines without CMake list semantics, which would mangle
  # scenario names containing ';', '[' or ']'
  string(REPLACE "\r\n" "\n" output "${output}")
  string(REPLACE "\r" "\n" output "${output}")
  string(APPEND output "\n")

  while(NOT output STREQUAL "")
    string(FIND "${output}" "\n" line_end)
    string(SUBSTRING "${output}" 0 ${line_end} line)
    math(EXPR line_end "${line_end} + 1")
    string(SUBSTRING "${output}" ${line_end} -1 output)

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
    _cucumber_escape_regex(escaped_test_name "${test_name}")
    _cucumber_bracket_argument(quoted_name_filter "^${escaped_test_name}$")

    set(full_test_name "${prefix}${test_name}${suffix}")
    _cucumber_bracket_argument(quoted_test_name "${full_test_name}")

    string(APPEND script "add_test(${quoted_test_name}${launcher_args_script} ${quoted_executable}${extra_args_script} [==[--name]==] ${quoted_name_filter})\n")
    string(APPEND script "set_tests_properties(${quoted_test_name} PROPERTIES WORKING_DIRECTORY ${quoted_working_dir})\n")

    if(NOT "${source_location}" STREQUAL "")
      _cucumber_bracket_argument(quoted_source_location "${source_location}")
      string(APPEND script "set_tests_properties(${quoted_test_name} PROPERTIES DEF_SOURCE_LINE ${quoted_source_location})\n")
    endif()

    if(NOT "${properties_script}" STREQUAL "")
      string(APPEND script "set_tests_properties(${quoted_test_name} PROPERTIES${properties_script})\n")
    endif()

    list(APPEND tests "${full_test_name}")
  endwhile()

  if(NOT "${arg_TEST_LIST}" STREQUAL "")
    _cucumber_bracket_argument(quoted_test_list "${arg_TEST_LIST}")
    _cucumber_bracket_argument(quoted_tests "${tests}")
    string(APPEND script "set(${quoted_test_list} ${quoted_tests})\n")
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

cmake_policy(POP)
