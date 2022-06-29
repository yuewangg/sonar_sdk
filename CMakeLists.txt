cmake_minimum_required(VERSION 3.10.0)
project(sonar_sdk)

# Determine if the BLUEVIEW_SDK_ROOT has been set
message("==================================")

# Include the BLUEVIW SDK header files
include_directories(/root/Documents/bvtsdk/include)

# Set the Blueview SDK libraries
set(BLUEVIEW_SDK_LIBS /root/Documents/bvtsdk/lib/libbvtsdk.so)



message("=======>")
message("${OpenCV_INCLUDE_DIRS}")

set( LIB_NAME "sdk_blueview" )

find_package(OpenCV REQUIRED)
find_package(Eigen3 REQUIRED)

# 输入 cmake -DDEBUG=OFF/ON .. 控制程序不定义/定义宏TEST，
if(DEBUG)
    add_definitions(-DDEBUG)
endif(DEBUG)

include_directories(${EIGEN3_INCLUDE_DIRS} ${OpenCV_INCLUDE_DIRS} "include" )
link_directories(${CMAKE_SOURCE_DIR}/lib /root/Documents/bvtsdk/lib)

add_executable(sonar_connect                 
                src/sonar_connect.cc
                src/udp.cc                      
)

add_executable(sonar_read                 
                src/sonar_read.cc
                src/udp.cc                      
)

target_link_libraries(sonar_read  yaml-cpp bvtsdk ${OpenCV_LIBS})

target_link_libraries(sonar_connect  yaml-cpp bvtsdk ${OpenCV_LIBS})
