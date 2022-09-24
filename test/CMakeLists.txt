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

if(BUILD_TEST)
  add_executable(unarr-test
                main.c)
  target_link_libraries(unarr-test unarr)

  function(run_test file)
    add_test(NAME ${file} COMMAND unarr-test ${CMAKE_CURRENT_SOURCE_DIR}/corpus/${file})
  endfunction(run_test file)

  # Integration tests - store (no compression)
  run_test(integration/lipsum_rar4_store.rar)
  run_test(integration/lipsum.tar)
  run_test(integration/lipsum_zip_copy.zip)
  run_test(integration/lipsum_7z_store.7z)

endif()

if (BUILD_UNIT_TESTS)
  add_executable(crc32-test
    crc32-test.c)
    #../common/crc32.c)
  target_link_libraries(crc32-test cmocka)

  add_test(crc32 crc32-test)
endif()
