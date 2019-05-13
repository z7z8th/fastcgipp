cmake_minimum_required(VERSION 3.7)
project(fastcgi++ VERSION 3.1 LANGUAGES CXX)

string(TIMESTAMP BUILD_TIME UTC)

# All our starting point lists
set(SRC_FILES
    "src/log.cpp"
    "src/block.cpp"
    "src/http.cpp"
    "src/protocol.cpp"
    "src/sockets.cpp"
    "src/transceiver.cpp"
    "src/fcgistreambuf.cpp"
    "src/webstreambuf.cpp"
    "src/request.cpp"
    "src/manager.cpp"
    "src/address.cpp"
    "src/mailer.cpp"
    "src/email.cpp")
set(TESTS
    "protocol"
    "http"
    "sockets"
    "transceiver"
    "fcgistreambuf")
set(EXAMPLES
    "helloworld"
    "echo"
    "gnu"
    "sessions"
    "email"
    "timer"
    "multi_uri_router")

# Set up our log level for fastcgi++/log.hpp
if(NOT LOG_LEVEL)
    if(CMAKE_BUILD_TYPE STREQUAL "RELEASE")
        set(LOG_LEVEL 1)
    elseif(CMAKE_BUILD_TYPE STREQUAL "DEBUG")
        set(LOG_LEVEL 4)
    else()
        set(LOG_LEVEL 2)
    endif()
endif()

message("*** CMAKE_BUILD_TYPE " ${CMAKE_BUILD_TYPE})
message("*** LOG_LEVEL " ${LOG_LEVEL})

# We'll need this stuff for sockets stuff
if(CMAKE_SYSTEM_NAME STREQUAL "Linux")
    set(SYSTEM "LINUX")
elseif(UNIX)
    set(SYSTEM "UNIX")
elseif(WIN32)
    message(FATAL_ERROR "Windows systems not supported")
    set(SYSTEM "WINDOWS")
else()
    message(FATAL_ERROR "Unknown operating system")
endif()


# Set compile flags for gcc and clang
if(UNIX)
    set(CMAKE_THREAD_PREFER_PTHREAD ON)
endif()

#Set RPATH
if(APPLE)
    if(CMAKE_BUILD_TYPE MATCHES "Debug")
        set(DEFAULT_FASTCGIPP_SKIP_RPATH OFF)
    else()
        set(DEFAULT_FASTCGIPP_SKIP_RPATH ON)
    endif()
    option(FASTCGIPP_SKIP_RPATH "Skip setting up RPATH to full" ${DEFAULT_FASTCGIPP_SKIP_RPATH})
    if(NOT FASTCGIPP_SKIP_RPATH)
        set(CMAKE_SKIP_BUILD_RPATH FALSE)
        set(CMAKE_BUILD_WITH_INSTALL_RPATH FALSE)
        set(CMAKE_INSTALL_RPATH "${CMAKE_INSTALL_PREFIX}/lib")
        set(CMAKE_INSTALL_RPATH_USE_LINK_PATH TRUE)
        list(FIND CMAKE_PLATFORM_IMPLICIT_LINK_DIRECTORIES "${CMAKE_INSTALL_PREFIX}/lib" isSystemDir)
        if("${isSystemDir}" STREQUAL "-1")
            set(CMAKE_INSTALL_RPATH "${CMAKE_INSTALL_PREFIX}/lib")
        endif()
    endif()
endif()

# Our configuration
configure_file(
    "${CMAKE_CURRENT_SOURCE_DIR}/include/config.hpp.in"
    "${CMAKE_CURRENT_BINARY_DIR}/include/fastcgi++/config.hpp" @ONLY)

# Are we worrying about SQL?
option(SQL "Set to ON to build and install the SQL components" OFF)
if(SQL)
    find_package(PostgreSQL REQUIRED)
    list(APPEND SRC_FILES "src/parameters.cpp")
    list(APPEND SRC_FILES "src/results.cpp")
    list(APPEND SRC_FILES "src/connection.cpp")
    list(APPEND EXAMPLES "sql")
endif(SQL)

# Build the library itself
option(BUILD_STATIC_LIBS "Set to ON to build and install only the static library" OFF)
if(BUILD_STATIC_LIBS)
    add_library(fastcgipp STATIC ${SRC_FILES})
else(BUILD_STATIC_LIBS)
    add_library(fastcgipp SHARED ${SRC_FILES})
endif(BUILD_STATIC_LIBS)

add_library(Fastcgipp::fastcgipp ALIAS fastcgipp)

set_target_properties(fastcgipp PROPERTIES VERSION ${PROJECT_VERSION}
                                           SOVERSION ${PROJECT_VERSION_MAJOR})

find_package(Threads REQUIRED)
target_link_libraries(fastcgipp PUBLIC Threads::Threads)
#target_compile_features(fastcgipp PRIVATE cxx_std_14)
set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED on)
set(CMAKE_CXX_EXTENSIONS off)
if(UNIX)
    if(CMAKE_CXX_COMPILER_ID MATCHES "GNU" OR CMAKE_CXX_COMPILER_ID MATCHES "Clang")
        target_compile_options(fastcgipp PRIVATE -Wall -Werror -fno-omit-frame-pointer)
    endif()
    if(CMAKE_BUILD_TYPE MATCHES "Debug")
        target_compile_options(fastcgipp PRIVATE -g -O0 -rdynamic)
    endif()
endif(UNIX)

include(GNUInstallDirs)
target_include_directories(fastcgipp PUBLIC
    $<INSTALL_INTERFACE:${CMAKE_INSTALL_LIBDIR}>
    $<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR}/include>
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>)

set(INSTALL_CONFIGDIR ${CMAKE_INSTALL_LIBDIR}/cmake/fastcgi++)

install(TARGETS fastcgipp
    EXPORT FastcgippTargets
    ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR})

install(EXPORT FastcgippTargets
    FILE FastcgippTargets.cmake
    NAMESPACE Fastcgipp::
    DESTINATION ${INSTALL_CONFIGDIR})

GNUInstallDirs_get_absolute_install_dir(FASTCGIPP_INCLUDE_DIR CMAKE_INSTALL_INCLUDEDIR)

#Create a ConfigVersion.cmake file
include(CMakePackageConfigHelpers)
write_basic_package_version_file(
    ${CMAKE_CURRENT_BINARY_DIR}/FastcgippConfigVersion.cmake
    COMPATIBILITY SameMajorVersion)

configure_package_config_file(${CMAKE_CURRENT_LIST_DIR}/cmake/FastcgippConfig.cmake.in
    ${CMAKE_CURRENT_BINARY_DIR}/FastcgippConfig.cmake
    INSTALL_DESTINATION ${INSTALL_CONFIGDIR}
    PATH_VARS FASTCGIPP_INCLUDE_DIR)

install(FILES
    ${CMAKE_CURRENT_BINARY_DIR}/FastcgippConfig.cmake
    ${CMAKE_CURRENT_BINARY_DIR}/FastcgippConfigVersion.cmake
    DESTINATION ${INSTALL_CONFIGDIR})

install(DIRECTORY
	"${CMAKE_CURRENT_SOURCE_DIR}/include/fastcgi++"
	DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
    PATTERN "sql" EXCLUDE)

if(SQL)
    target_link_libraries(fastcgipp PUBLIC ${PostgreSQL_LIBRARIES})
    target_include_directories(fastcgipp PRIVATE ${PostgreSQL_INCLUDE_DIRS})
    install(DIRECTORY
        "${CMAKE_CURRENT_SOURCE_DIR}/include/fastcgi++/sql"
        DESTINATION "${CMAKE_INSTALL_INCLUDEDIR}/fastcgi++")
endif(SQL)

# Install the config header file
install(FILES
    "${CMAKE_CURRENT_BINARY_DIR}/include/fastcgi++/config.hpp"
    DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/fastcgi++)

# All the test stuff
enable_testing()
foreach(UNITTEST IN LISTS TESTS)
    add_executable(${UNITTEST}_test EXCLUDE_FROM_ALL tests/${UNITTEST}.cpp)
    target_link_libraries(${UNITTEST}_test PRIVATE Fastcgipp::fastcgipp)
    add_test("${UNITTEST}" ${UNITTEST}_test)
    list(APPEND TEST_TARGET ${UNITTEST}_test)
endforeach()
if(SQL)
    configure_file(
        "${CMAKE_CURRENT_SOURCE_DIR}/tests/sql.sh.in"
        "${CMAKE_CURRENT_BINARY_DIR}/sql_test.sh" @ONLY)
    add_executable(sql_test EXCLUDE_FROM_ALL tests/sql.cpp)
    target_link_libraries(sql_test PRIVATE Fastcgipp::fastcgipp)
    add_test("sql" sql_test.sh)
    list(APPEND TEST_TARGET sql_test)
    target_include_directories(sql_test PRIVATE ${PostgreSQL_INCLUDE_DIRS})
endif(SQL)
add_custom_target(tests DEPENDS ${TEST_TARGET})

# Examples
foreach(EXAMPLE IN LISTS EXAMPLES)
    add_executable(${EXAMPLE}.fcgi EXCLUDE_FROM_ALL examples/${EXAMPLE}.cpp)
    if(CMAKE_BUILD_TYPE MATCHES "Debug")
        target_compile_options(${EXAMPLE}.fcgi PRIVATE -g -O0 -rdynamic)
    endif()
    target_link_libraries(${EXAMPLE}.fcgi PRIVATE Fastcgipp::fastcgipp)
    list(APPEND EXAMPLE_TARGETS ${EXAMPLE}.fcgi)
endforeach()
add_custom_target(examples DEPENDS ${EXAMPLE_TARGETS})

# And finally the documentation
find_package(Doxygen)
if(DOXYGEN_FOUND)
    configure_file(
        "${CMAKE_CURRENT_SOURCE_DIR}/doc/Doxyfile.in"
        "${CMAKE_CURRENT_BINARY_DIR}/Doxyfile" @ONLY)
    add_custom_target(
        doc
        "${DOXYGEN_EXECUTABLE}" "${CMAKE_CURRENT_BINARY_DIR}/Doxyfile"
        WORKING_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}"
        COMMENT "Generating API documentation with Doxygen" VERBATIM)
    install(
        DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/doc/"
        DESTINATION ${CMAKE_INSTALL_DOCDIR}
        OPTIONAL)
endif(DOXYGEN_FOUND)
