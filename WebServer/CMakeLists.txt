cmake_minimum_required(VERSION 3.23)
 project(WebServer)

 set(CMAKE_CXX_STANDARD 20)
 set(CMAKE_CXX_STANDARD_REQUIRED ON)
 set(CMAKE_CXX_EXTENSIONS OFF)

 include(${CMAKE_BINARY_DIR}/conanbuildinfo.cmake)
 conan_basic_setup()

 file(GLOB_RECURSE SOURCES RELATIVE ${CMAKE_SOURCE_DIR} "server/VideoGamePublishersServer/*")
 
 add_executable(Server ${SOURCES} ${VIEWSRC})
 target_link_libraries(Server ${CONAN_LIBS})