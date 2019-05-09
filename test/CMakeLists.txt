if(NOT FUZZER_TEST)
    return()
endif()

add_executable(fuzzer fuzzer.c)
target_link_libraries(fuzzer unarr )

file(MAKE_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/corpus)

add_test(NAME fuzzer_test 
    COMMAND fuzzer corpus
    WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR})
