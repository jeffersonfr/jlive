cmake_minimum_required (VERSION 3.0)

add_executable(${PROJECT_NAME}
  client.cpp
  configuration.cpp
  network.cpp
  requestparser.cpp
  server.cpp
  source.cpp
  main.cpp
)

install(TARGETS ${PROJECT_NAME} LIBRARY DESTINATION ${CMAKE_INSTALL_PREFIX}/bin)
