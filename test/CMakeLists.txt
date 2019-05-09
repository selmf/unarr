project (unarr-test C)

include(CTest)
include(ProcessorCount)
ProcessorCount(N)

if (BUILD_FUZZER)
  add_executable(fuzzer fuzzer.c)
  set_target_properties(fuzzer PROPERTIES LINK_FLAGS "${sanitize_opts}")
  target_link_libraries(fuzzer unarr)

  file(MAKE_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/corpus)
  file(MAKE_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/corpus/fuzzed)

  add_test(NAME fuzzer_test
          COMMAND fuzzer
            ${CMAKE_CURRENT_SOURCE_DIR}/corpus/fuzzed
            ${CMAKE_CURRENT_SOURCE_DIR}/corpus
            -jobs=${N})
endif()
