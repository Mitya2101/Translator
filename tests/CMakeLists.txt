set(TEST_DIR ${CMAKE_CURRENT_SOURCE_DIR}/program)
set(EXPECTED_DIR ${CMAKE_CURRENT_SOURCE_DIR}/expected)


file(GLOB TEST_FILES CONFIGURE_DEPENDS ${TEST_DIR}/*.txt)

foreach(test_file ${TEST_FILES})
    get_filename_component(test_name ${test_file} NAME_WE)

    add_test(
        NAME ${test_name}
        COMMAND sh -c "$<TARGET_FILE:MyLang> ${test_file} > ${CMAKE_CURRENT_SOURCE_DIR}/LangOutput/${test_name}.txt; 
        diff ${CMAKE_CURRENT_SOURCE_DIR}/LangOutput/${test_name}.txt ${EXPECTED_DIR}/${test_name}.txt"
    )

    set_tests_properties(${test_name} PROPERTIES WORKING_DIRECTORY ${CMAKE_BINARY_DIR})

    message(STATUS "Added test: ${test_name}")
endforeach()
