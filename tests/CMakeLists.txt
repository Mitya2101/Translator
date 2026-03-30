set(TEST_DIR ${CMAKE_CURRENT_SOURCE_DIR}/program)
set(EXPECTED_DIR ${CMAKE_CURRENT_SOURCE_DIR}/expected)
set(OUTPUT ${CMAKE_CURRENT_SOURCE_DIR})

file(GLOB TEST_FILES CONFIGURE_DEPENDS ${TEST_DIR}/*.txt)

foreach(test_file ${TEST_FILES})
    get_filename_component(test_name ${test_file} NAME_WE)

    add_test(
        NAME ${test_name}
        COMMAND sh -c "$<TARGET_FILE:MyLang> ${test_file} > ${OUTPUT}/actual_output.txt; diff ${OUTPUT}/actual_output.txt ${EXPECTED_DIR}/${test_name}.txt"
    )

    set_tests_properties(${test_name} PROPERTIES WORKING_DIRECTORY ${CMAKE_BINARY_DIR})

    message(STATUS "Added test: ${test_name}")
endforeach()
