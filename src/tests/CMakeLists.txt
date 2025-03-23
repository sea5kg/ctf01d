file(GLOB ALL_TESTS
     "test_*.cpp"
)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY tests)

# add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/../3rdparty/sqlite3 ${CMAKE_CURRENT_BINARY_DIR}/sqlite3)
list (APPEND WSJCPP_LIBRARIES sqlite3-static)
list (APPEND WSJCPP_INCLUDE_DIRS "${PROJECT_BINARY_DIR}/../3rdparty/sqlite3/src")


list (APPEND TEST_CTF01D_SOURCES "${CMAKE_CURRENT_SOURCE_DIR}/../employees/employ_flags.cpp")

# wsjcpp-core
list (APPEND WSJCPP_INCLUDE_DIRS "${CMAKE_SOURCE_DIR}/../../src.wsjcpp/wsjcpp_core/")
list (APPEND TEST_CTF01D_SOURCES "${CMAKE_CURRENT_SOURCE_DIR}/../../src.wsjcpp/wsjcpp_core/wsjcpp_core.cpp")

# nlohmann/json
list (APPEND WSJCPP_INCLUDE_DIRS "${CMAKE_CURRENT_SOURCE_DIR}/../../src.wsjcpp/nlohmann_json/")

# wsjcpp-employees
list (APPEND WSJCPP_INCLUDE_DIRS "${CMAKE_CURRENT_SOURCE_DIR}/../../src.wsjcpp/wsjcpp_employees/")
list (APPEND TEST_CTF01D_SOURCES "${CMAKE_CURRENT_SOURCE_DIR}/../../src.wsjcpp/wsjcpp_employees/wsjcpp_employees.cpp")

foreach(_TEST ${ALL_TESTS})
    get_filename_component(TESTNAME ${_TEST} NAME_WE)
    add_executable(${TESTNAME}
        ${_TEST}
        ${TEST_CTF01D_SOURCES}
    )
    target_link_libraries(${TESTNAME} -lpthread ${WSJCPP_LIBRARIES})

    add_test(
      NAME ${TESTNAME}
      COMMAND $<TARGET_FILE:${TESTNAME}>
    )
    message(${CMAKE_CURRENT_BINARY_DIR}/tests)
    set_target_properties (${TESTNAME} PROPERTIES RUNTIME_OUTPUT_DIRECTORY tests)
endforeach()