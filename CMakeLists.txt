#*********************************************************************************
#          Project
#  _______ _________ _______  _______  __    ______
# (  ____ \\__   __/(  ____ \(  __   )/  \  (  __  \
# | (    \/   ) (   | (    \/| (  )  |\/) ) | (  \  )
# | |         | |   | (__    | | /   |  | | | |   ) |
# | |         | |   |  __)   | (/ /) |  | | | |   | |
# | |         | |   | (      |   / | |  | | | |   ) |
# | (____/\   | |   | )      |  (__) |__) (_| (__/  )
# (_______/   )_(   |/       (_______)\____/(______/
#
# MIT License
# Copyright (c) 2018-2025 Evgenii Sopov
#
# Permission is hereby granted, free of charge, to any person obtaining a copy
# of this software and associated documentation files (the "Software"), to deal
# in the Software without restriction, including without limitation the rights
# to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
# copies of the Software, and to permit persons to whom the Software is
# furnished to do so, subject to the following conditions:
#
# The above copyright notice and this permission notice shall be included in all
# copies or substantial portions of the Software.
#
# THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
# IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
# FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
# AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
# LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
# OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
# SOFTWARE.
#
##################################################################################
# ctf01d CMake script

cmake_minimum_required(VERSION 3.18)

project(ctf01d)

include(${CMAKE_CURRENT_SOURCE_DIR}/src.wsjcpp/CMakeLists.txt)

#### BEGIN_WSJCPP_APPEND
#### END_WSJCPP_APPEND

set(CMAKE_CXX_STANDARD 17)
set(EXECUTABLE_OUTPUT_PATH ${ctf01d_SOURCE_DIR})
# set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/cmake/Modules/")

# Sources

# include header dirs
list (APPEND WSJCPP_INCLUDE_DIRS "src")
list (APPEND WSJCPP_INCLUDE_DIRS "src/argument_processors")
list (APPEND WSJCPP_INCLUDE_DIRS "src/checker")
list (APPEND WSJCPP_INCLUDE_DIRS "src/scoreboard")
list (APPEND WSJCPP_INCLUDE_DIRS "src/employees")
list (APPEND WSJCPP_INCLUDE_DIRS "src/store")
list (APPEND WSJCPP_INCLUDE_DIRS "src/3rdparty/sqlite3/src")

# argument processors
list (APPEND WSJCPP_SOURCES "./src/argument_processors/argument_processor_check.cpp")
list (APPEND WSJCPP_SOURCES "./src/argument_processors/argument_processor_clean.cpp")
list (APPEND WSJCPP_SOURCES "./src/argument_processors/argument_processor_ctf01d_main.cpp")
list (APPEND WSJCPP_SOURCES "./src/argument_processors/argument_processor_services.cpp")
list (APPEND WSJCPP_SOURCES "./src/argument_processors/argument_processor_start.cpp")
list (APPEND WSJCPP_SOURCES "./src/argument_processors/argument_processor_teams.cpp")
list (APPEND WSJCPP_SOURCES "./src/argument_processors/argument_processor_version.cpp")
list (APPEND WSJCPP_SOURCES "./src/argument_processors/argument_processor_web_test.cpp")

# employees
list (APPEND WSJCPP_SOURCES "./src/employees/employ_team_logos.cpp")
list (APPEND WSJCPP_SOURCES "./src/employees/employ_flags.cpp")
list (APPEND WSJCPP_SOURCES "./src/employees/employ_scoreboard.cpp")
list (APPEND WSJCPP_SOURCES "./src/employees/employ_config.cpp")
list (APPEND WSJCPP_SOURCES "./src/employees/employ_database.cpp")

# checker
list (APPEND WSJCPP_SOURCES "src/checker/dorunchecker.cpp")
list (APPEND WSJCPP_SOURCES "src/checker/service_checker_thread.cpp")

# http_server
list (APPEND WSJCPP_SOURCES "src/ctf01d_http_server.cpp")

# scoreboard
list (APPEND WSJCPP_SOURCES "src/scoreboard/ctf01d_scoreboard.cpp")

# store
list (APPEND WSJCPP_SOURCES "src/store/ctf01d_store.cpp")
list (APPEND WSJCPP_SOURCES "src/store/http_downloader.cpp")

# main
list (APPEND WSJCPP_SOURCES "src/main.cpp")

# ZLIB
# find_package( ZLIB REQUIRED )
# if ( ZLIB_FOUND )
#     list (APPEND WSJCPP_INCLUDE_DIRS ${ZLIB_INCLUDE_DIRS})
#     include_directories( ${ZLIB_INCLUDE_DIRS} )
#     list (APPEND WSJCPP_LIBRARIES ${ZLIB_LIBRARIES})
# endif( ZLIB_FOUND )

# # CURL
FIND_PACKAGE(CURL)
IF(CURL_FOUND)
  list (APPEND WSJCPP_INCLUDE_DIRS ${CURL_INCLUDE_DIR})
  list (APPEND WSJCPP_LIBRARIES ${CURL_LIBRARIES})
ELSE(CURL_FOUND)
  MESSAGE(FATAL_ERROR "Could not find the CURL library and development files.")
ENDIF(CURL_FOUND)

# libhv
set(WITH_OPENSSL OFF CACHE BOOL "")
set(WITH_CURL OFF CACHE BOOL "")
# set(WITH_MQTT OFF CACHE BOOL "")
# set(WITH_EVPP OFF CACHE BOOL "")
set(BUILD_EXAMPLES OFF CACHE BOOL "")
# set(WITH_HTTP_CLIENT OFF CACHE BOOL "")
set(BUILD_SHARED OFF CACHE BOOL "")

add_subdirectory(src/3rdparty/libhv)
list (APPEND WSJCPP_LIBRARIES hv_static)
list (APPEND WSJCPP_INCLUDE_DIRS "${PROJECT_BINARY_DIR}/src/3rdparty/libhv/include/hv")

add_subdirectory(src/3rdparty/sqlite3)
# https://www.sqlite.org/threadsafe.html
# target_compile_definitions(sqlite3-static
#   PRIVATE
#     SQLITE_THREADSAFE=0
# )
list (APPEND WSJCPP_LIBRARIES sqlite3-static)
list (APPEND WSJCPP_INCLUDE_DIRS "${PROJECT_BINARY_DIR}/src/3rdparty/sqlite3/src")

include_directories(${WSJCPP_INCLUDE_DIRS})

add_executable (ctf01d ${WSJCPP_SOURCES})

target_link_libraries(ctf01d -lpthread ${WSJCPP_LIBRARIES} )

install(
    TARGETS
        ctf01d
    RUNTIME DESTINATION
        ${CMAKE_INSTALL_PREFIX}/bin
)

# enable testing functionality
enable_testing()
add_subdirectory(src/tests)
