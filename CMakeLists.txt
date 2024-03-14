


cmake_minimum_required(VERSION 3.14.0)
project(OpenCVProject)

set(CMAKE_CXX_STANDARD 17)
# set(CMAKE_VERBOSE_MAKEFILE ON)

set(OpenCV_DIR "C:/Users/user/Downloads/opencv/build")
find_package(OpenCV REQUIRED)
include_directories(${OpenCV_INCLUDE_DIRS})

# This is just to confirm that the include directory is correct, which it is
message(STATUS "Looking for OpenCV includes in: ${OpenCV_INCLUDE_DIRS}")

add_executable(OpenCVProject main.cpp)

target_link_libraries(OpenCVProject ${OpenCV_LIBS})


#C:\Program Files\Android\OpenCV-android-sdk\sdk\native\jni