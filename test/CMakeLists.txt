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

if(BUILD_SAMPLES)
  add_executable(unarr-test
                main.c)
  # add_dependencies(unarr-test unarr)
  target_link_libraries(unarr-test unarr)
endif()

if (BUILD_UNIT_TESTS)
  add_executable(crc32-test
    crc32-test.c)
    #../common/crc32.c)
  target_link_libraries(crc32-test cmocka)

  add_test(crc32 crc32-test)
endif()
