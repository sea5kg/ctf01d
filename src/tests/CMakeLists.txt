file(GLOB ALL_TESTS
     "test_*.cpp"
)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY tests)

# add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/../third_party/sqlite3 ${CMAKE_CURRENT_BINARY_DIR}/sqlite3)
# list (APPEND WSJCPP_LIBRARIES sqlite3-static)
# list (APPEND TEST_CTF01D_INCLUDE_DIRS "${PROJECT_BINARY_DIR}/../third_party/sqlite3/src")

list (APPEND TEST_CTF01D_INCLUDE_DIRS "${CMAKE_CURRENT_SOURCE_DIR}/../ctf01d/objects/")
list (APPEND TEST_CTF01D_SOURCES "${CMAKE_CURRENT_SOURCE_DIR}/../ctf01d/objects/ctf01d_flag.cpp")
list (APPEND TEST_CTF01D_SOURCES "${CMAKE_CURRENT_SOURCE_DIR}/../ctf01d/objects/ctf01d_formulas_for_points.cpp")
list (APPEND TEST_CTF01D_SOURCES "${CMAKE_CURRENT_SOURCE_DIR}/../employees/employ_flags.cpp")

# wsjcpp-core
list (APPEND TEST_CTF01D_INCLUDE_DIRS "${CMAKE_SOURCE_DIR}/../../src.wsjcpp/wsjcpp_core/")
list (APPEND TEST_CTF01D_SOURCES "${CMAKE_CURRENT_SOURCE_DIR}/../../src.wsjcpp/wsjcpp_core/wsjcpp_core.cpp")

# nlohmann/json
list (APPEND TEST_CTF01D_INCLUDE_DIRS "${CMAKE_CURRENT_SOURCE_DIR}/../../src.wsjcpp/nlohmann_json/")

# wsjcpp-employees
list (APPEND TEST_CTF01D_INCLUDE_DIRS "${CMAKE_CURRENT_SOURCE_DIR}/../../src.wsjcpp/wsjcpp_employees/")
list (APPEND TEST_CTF01D_SOURCES "${CMAKE_CURRENT_SOURCE_DIR}/../../src.wsjcpp/wsjcpp_employees/wsjcpp_employees.cpp")

include_directories(${TEST_CTF01D_INCLUDE_DIRS})

foreach(_TEST ${ALL_TESTS})
    get_filename_component(TEST_NAME ${_TEST} NAME_WE)
    add_executable(${TEST_NAME}
        ${_TEST}
        ${TEST_CTF01D_SOURCES}
    )
    target_link_libraries(${TEST_NAME} -lpthread ${WSJCPP_LIBRARIES})

    add_test(
      NAME ${TEST_NAME}
      COMMAND $<TARGET_FILE:${TEST_NAME}>
    )
    message(${CMAKE_CURRENT_BINARY_DIR}/tests)
    set_target_properties (${TEST_NAME} PROPERTIES RUNTIME_OUTPUT_DIRECTORY tests)
endforeach()
