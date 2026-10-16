# Distributed under the Apache-2.0 License.
# Defines cucumber_discover_tests for automatic scenario discovery with CTest.

# Scope policy settings to this file so including it never alters the
# caller's policies; functions defined here record these settings.
cmake_policy(PUSH)

if(POLICY CMP0174)
  cmake_policy(SET CMP0174 NEW)
endif()

include("${CMAKE_CURRENT_LIST_DIR}/cucumber_add_tests.cmake")

function(cucumber_discover_tests target)
  set(options "")
  set(oneValueArgs
    TEST_PREFIX
    TEST_SUFFIX
    WORKING_DIRECTORY
    TEST_LIST
    DISCOVERY_TIMEOUT
    DISCOVERY_MODE
  )
  set(multiValueArgs
    EXTRA_ARGS
    PROPERTIES
  )
  cmake_parse_arguments(PARSE_ARGV 1 arg
    "${options}" "${oneValueArgs}" "${multiValueArgs}"
  )

  if(NOT arg_WORKING_DIRECTORY)
    set(arg_WORKING_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}")
  endif()
  if(NOT arg_TEST_LIST)
    set(arg_TEST_LIST ${target}_TESTS)
  endif()
  if(NOT arg_DISCOVERY_TIMEOUT)
    set(arg_DISCOVERY_TIMEOUT 30)
  endif()
  if(NOT arg_DISCOVERY_MODE)
    set(arg_DISCOVERY_MODE "POST_BUILD")
  endif()

  get_property(
    has_counter
    TARGET ${target}
    PROPERTY CTEST_DISCOVERED_TEST_COUNTER
    SET
  )
  if(has_counter)
    get_property(
      counter
      TARGET ${target}
      PROPERTY CTEST_DISCOVERED_TEST_COUNTER
    )
    math(EXPR counter "${counter} + 1")
  else()
    set(counter 1)
  endif()
  set_property(
    TARGET ${target}
    PROPERTY CTEST_DISCOVERED_TEST_COUNTER
    ${counter}
  )

  # Define rule to generate test list for test executable
  set(ctest_file_base "${CMAKE_CURRENT_BINARY_DIR}/${target}[${counter}]")
  set(ctest_include_file "${ctest_file_base}_include.cmake")
  set(ctest_tests_file "${ctest_file_base}_tests.cmake")

  get_property(test_launcher
    TARGET ${target}
    PROPERTY TEST_LAUNCHER
  )
  if(CMAKE_CROSSCOMPILING)
    get_property(crosscompiling_emulator
      TARGET ${target}
      PROPERTY CROSSCOMPILING_EMULATOR
    )
  endif()

  if(test_launcher AND crosscompiling_emulator)
    set(test_executor "${test_launcher}" "${crosscompiling_emulator}")
  elseif(test_launcher)
    set(test_executor "${test_launcher}")
  elseif(crosscompiling_emulator)
    set(test_executor "${crosscompiling_emulator}")
  else()
    set(test_executor "")
  endif()

  set(add_tests_script "${CMAKE_CURRENT_FUNCTION_LIST_DIR}/cucumber_add_tests.cmake")

  get_property(GENERATOR_IS_MULTI_CONFIG GLOBAL
    PROPERTY GENERATOR_IS_MULTI_CONFIG
  )

  if(arg_DISCOVERY_MODE STREQUAL "POST_BUILD")
    if(GENERATOR_IS_MULTI_CONFIG)
      set(ctest_tests_file "${ctest_file_base}_tests-$<CONFIG>.cmake")
      set(ctest_tests_include_file "${ctest_file_base}_tests-\${CTEST_CONFIGURATION_TYPE}.cmake")
    else()
      set(ctest_tests_include_file "${ctest_tests_file}")
    endif()

    add_custom_command(
      TARGET ${target} POST_BUILD
      BYPRODUCTS "${ctest_tests_file}"
      COMMAND "${CMAKE_COMMAND}"
              -D "TEST_TARGET=${target}"
              -D "TEST_EXECUTABLE=$<TARGET_FILE:${target}>"
              -D "TEST_EXECUTOR=${test_executor}"
              -D "TEST_WORKING_DIR=${arg_WORKING_DIRECTORY}"
              -D "TEST_EXTRA_ARGS=${arg_EXTRA_ARGS}"
              -D "TEST_PROPERTIES=${arg_PROPERTIES}"
              -D "TEST_PREFIX=${arg_TEST_PREFIX}"
              -D "TEST_SUFFIX=${arg_TEST_SUFFIX}"
              -D "TEST_LIST=${arg_TEST_LIST}"
              -D "CTEST_FILE=${ctest_tests_file}"
              -D "TEST_DISCOVERY_TIMEOUT=${arg_DISCOVERY_TIMEOUT}"
              -P "${add_tests_script}"
      VERBATIM
    )

    file(WRITE "${ctest_include_file}"
      "if(EXISTS \"${ctest_tests_include_file}\")\n"
      "  include(\"${ctest_tests_include_file}\")\n"
      "else()\n"
      "  add_test(${target}_NOT_BUILT ${target}_NOT_BUILT)\n"
      "endif()\n"
    )
  elseif(arg_DISCOVERY_MODE STREQUAL "PRE_TEST")
    if(GENERATOR_IS_MULTI_CONFIG)
      set(ctest_tests_file "${ctest_file_base}_tests-$<CONFIG>.cmake")
    endif()

    foreach(name IN ITEMS WORKING_DIRECTORY EXTRA_ARGS PROPERTIES TEST_PREFIX TEST_SUFFIX TEST_LIST)
      _cucumber_bracket_argument(quoted_${name} "${arg_${name}}")
    endforeach()

    # Always rediscover so that changes to feature files are picked up
    string(CONCAT ctest_include_content
      "if(EXISTS \"$<TARGET_FILE:${target}>\")"                                  "\n"
      "  include(\"${add_tests_script}\")"                                       "\n"
      "  cucumber_discover_tests_impl("                                          "\n"
      "    TEST_TARGET"            " [==[${target}]==]"                          "\n"
      "    TEST_EXECUTABLE"        " [==[$<TARGET_FILE:${target}>]==]"           "\n"
      "    TEST_EXECUTOR"          " [==[${test_executor}]==]"                   "\n"
      "    TEST_WORKING_DIR"       " ${quoted_WORKING_DIRECTORY}"                "\n"
      "    TEST_EXTRA_ARGS"        " ${quoted_EXTRA_ARGS}"                       "\n"
      "    TEST_PROPERTIES"        " ${quoted_PROPERTIES}"                       "\n"
      "    TEST_PREFIX"            " ${quoted_TEST_PREFIX}"                      "\n"
      "    TEST_SUFFIX"            " ${quoted_TEST_SUFFIX}"                      "\n"
      "    TEST_LIST"              " ${quoted_TEST_LIST}"                        "\n"
      "    CTEST_FILE"             " [==[${ctest_tests_file}]==]"                "\n"
      "    TEST_DISCOVERY_TIMEOUT" " [==[${arg_DISCOVERY_TIMEOUT}]==]"           "\n"
      "  )"                                                                      "\n"
      "  include(\"${ctest_tests_file}\")"                                       "\n"
      "else()"                                                                   "\n"
      "  add_test(${target}_NOT_BUILT ${target}_NOT_BUILT)"                      "\n"
      "endif()"                                                                  "\n"
    )

    if(GENERATOR_IS_MULTI_CONFIG)
      file(GENERATE
        OUTPUT "${ctest_file_base}_include-$<CONFIG>.cmake"
        CONTENT "${ctest_include_content}"
      )
      file(WRITE "${ctest_include_file}"
        "include(\"${ctest_file_base}_include-\${CTEST_CONFIGURATION_TYPE}.cmake\")"
      )
    else()
      file(GENERATE
        OUTPUT "${ctest_include_file}"
        CONTENT "${ctest_include_content}"
      )
    endif()
  else()
    message(FATAL_ERROR "Unknown DISCOVERY_MODE: ${arg_DISCOVERY_MODE}")
  endif()

  set_property(DIRECTORY
    APPEND PROPERTY TEST_INCLUDE_FILES "${ctest_include_file}"
  )
endfunction()

cmake_policy(POP)
