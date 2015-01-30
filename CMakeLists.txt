cmake_minimum_required(VERSION 2.8.4)
project(neversnake)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")

set(SOURCE_FILES src/main.cpp src/image.cpp src/snake.cpp cabeceras/snake.h cabeceras/image.h texturas/snake.bmp texturas/apple.bmp cabeceras/rank.h src/rank.c src/asd.txt)
add_executable(neversnake ${SOURCE_FILES})

find_package(OpenGL REQUIRED)
find_package(GLUT REQUIRED)
include_directories( ${OPENGL_INCLUDE_DIRS}  ${GLUT_INCLUDE_DIRS} )

target_link_libraries(neversnake ${OPENGL_LIBRARIES} ${GLUT_LIBRARY} )