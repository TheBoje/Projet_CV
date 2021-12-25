cmake_minimum_required(VERSION 3.21)
project(TP1)

find_package( OpenCV REQUIRED )

set(CMAKE_CXX_STANDARD 14)

add_executable(TP1
        main.cpp)

target_link_libraries( TP1 ${OpenCV_LIBS} )

set(OpenCV_FOUND 1)