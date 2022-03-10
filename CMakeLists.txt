cmake_minimum_required (VERSION 3.0)

project (jlive)

set(CMAKE_CXX_STANDARD 20)

add_subdirectory(src)

install(FILES man/jlive.1 DESTINATION ${CMAKE_INSTALL_PREFIX}/man/man1/jlive.1)
install(DIRECTORY mlive DESTINATION /etc)
