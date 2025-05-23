cmake_minimum_required(VERSION 3.10.2)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

project(DroneControl)

add_executable(DroneControl
    src/drone-control.cpp
    src/drone-process.cpp

    src/sim/ns3-bridge.cpp
    src/sim/ns3-utils.cpp
    src/sim/world_parser.cpp

    src/libs/tinyxml2.cpp
)

find_package(MAVSDK REQUIRED)
find_package(Threads REQUIRED)

# Find NS-3
find_package(ns3 REQUIRED)

target_link_libraries(DroneControl
    MAVSDK::mavsdk
    Threads::Threads
)

include_directories(${DroneControl_SOURCE_DIR})

if(NOT MSVC)
    add_compile_options(DroneControl PRIVATE -Wall -Wextra)
else()
    add_compile_options(DroneControl PRIVATE -W2)
endif()

# Windows with MinGW Installations
if( ${CMAKE_SYSTEM_NAME} MATCHES "Windows" AND MINGW )
    # if working on Windows but not in the lab
    # update the include directory location
    include_directories("Z:/include")
    # update the lib directory location
    target_link_directories(${PROJECT_NAME} PUBLIC "Z:/lib")
# OS X Installations
elseif( APPLE AND ${CMAKE_SYSTEM_NAME} MATCHES "Darwin" )
    # Specify the directory where your libraries are located
    set(LIB_DIR "/usr/local/lib")
    # update the include directory location
    include_directories("/usr/local/include")

    file(GLOB MY_LIBS "${LIB_DIR}/libns3*dylib")
    # update the lib directory location
    target_link_directories(${PROJECT_NAME} PUBLIC "${LIB_DIR}")
    target_link_libraries(${PROJECT_NAME} ${MY_LIBS})
# Blanket *nix Installations
elseif( UNIX AND ${CMAKE_SYSTEM_NAME} MATCHES "Linux" )
    set(LIB_DIR "/usr/local/lib")
    # update the include directory location
    include_directories("/usr/local/include")
    # update the lib directory location
    target_link_directories(${PROJECT_NAME} PUBLIC "/usr/local/lib")
    file(GLOB MY_LIBS "${LIB_DIR}/libns3.4*so")
    target_link_libraries(${PROJECT_NAME} ${MY_LIBS})
endif()
