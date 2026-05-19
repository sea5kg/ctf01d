##################################################################################
#           Project
#   _______ _________ _______  _______  __    ______
#  (  ____ \\__   __/(  ____ \(  __   )/  \  (  __  \
#  | (    \/   ) (   | (    \/| (  )  |\/) ) | (  \  )
#  | |         | |   | (__    | | /   |  | | | |   ) |
#  | |         | |   |  __)   | (/ /) |  | | | |   | |
#  | |         | |   | (      |   / | |  | | | |   ) |
#  | (____/\   | |   | )      |  (__) |__) (_| (__/  )
#  (_______/   )_(   |/       (_______)\____/(______/
#
# MIT License
#
# Copyright (c) 2018-2026 Evgenii Sopov
#
# Permission is hereby granted, free of charge, to any person obtaining a copy
# of this software and associated documentation files (the "Software"), to deal
# in the Software without restriction, including without limitation the rights
# to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
# copies of the Software, and to permit persons to whom the Software is
# furnished to do so, subject to the following conditions:
#
# The above copyright notice and this permission notice shall be included in
# all copies or substantial portions of the Software.
#
# THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
# IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
# FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
# AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
# LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
# OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
# SOFTWARE.
#
# Original repository: https://github.com/sea5kg/ctf01d
#
##################################################################################

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
