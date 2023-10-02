cmake_minimum_required (VERSION 3.0)

if (CMAKE_SOURCE_DIR STREQUAL CMAKE_BINARY_DIR)
  message(FATAL_ERROR "In-source build is not allowed")
endif()

project(jlive
  LANGUAGES CXX
  HOMEPAGE_URL https://github.com/jeffersonfr/jlive
  DESCRIPTION "Live server"
  VERSION 1.0.0
)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

enable_testing()

add_subdirectory(src)

install(FILES man/jlive.1 DESTINATION ${CMAKE_INSTALL_PREFIX}/man/man1/jlive.1)
install(DIRECTORY mlive DESTINATION /etc)
