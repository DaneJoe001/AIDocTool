cmake_minimum_required(VERSION 3.16)

project(AIDocTools LANGUAGES CXX)

if(MSVC)
    add_compile_options(/Zc:__cplusplus /permissive-)
    set(CMAKE_PREFIX_PATH "D:/Application/Qt6.5/6.5.3/msvc2019_64")
endif()

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

set(RESOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/resource")

find_package(Qt6 REQUIRED COMPONENTS Core Widgets REQUIRED SerialPort Concurrent)

file(GLOB_RECURSE SOURCES "source/*.cpp" "include/*.hpp" "include/*.h")
file(GLOB_RECURSE RESOURCES "resource/*.qrc")

add_executable(aidoctools ${SOURCES} ${RESOURCES})

target_compile_definitions(aidoctools PRIVATE RESOURCE_DIR="${RESOURCE_DIR}")
target_link_libraries(aidoctools PRIVATE Qt6::Core Qt6::Widgets Qt6::SerialPort Qt6::Concurrent)
target_include_directories(aidoctools PRIVATE include)

#设置qDebug在终端输出
set_target_properties(aidoctools PROPERTIES
    WIN32_EXECUTABLE OFF
    MACOSX_BUNDLE ON
)
