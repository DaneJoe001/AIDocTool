cmake_minimum_required(VERSION 3.16)

project(AIDocTools LANGUAGES CXX)

if(MSVC)
    add_compile_options(/Zc:__cplusplus /permissive-)
    set(CMAKE_PREFIX_PATH "D:/personal_app/app_qt/qt_6.9/6.9.1/msvc2022_64")
    else()
    set(CMAKE_PREFIX_PATH "D:/personal_app/app_qt/qt_6.9/6.9.1/mingw_64")
endif()


set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# 设置发布模式
set(CMAKE_BUILD_TYPE Release)
if(MSVC)
    set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} /O2 /GL /Gy /MT")
else()
    set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -O3")
endif()

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

set(RESOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/resource")

find_package(Qt6 REQUIRED COMPONENTS Core Widgets REQUIRED Concurrent)

file(GLOB_RECURSE SOURCES "source/*.cpp" "include/*.hpp" "include/*.h")
file(GLOB_RECURSE RESOURCES "resource/*.qrc")

# 排除测试文件，避免重复编译
list(FILTER SOURCES EXCLUDE REGEX ".*test_.*\\.cpp$")

# 手动添加新的源文件
list(APPEND SOURCES "source/filterrulelistwidget.cpp" "include/filterrulelistwidget.h")

# 添加Windows资源文件
if(WIN32)
    set(APP_ICON_RESOURCE_WINDOWS "${CMAKE_CURRENT_SOURCE_DIR}/resource/app_icon.rc")
endif()

add_executable(aidoctools ${SOURCES} ${RESOURCES} ${APP_ICON_RESOURCE_WINDOWS})

target_compile_definitions(aidoctools PRIVATE RESOURCE_DIR="${RESOURCE_DIR}")
target_link_libraries(aidoctools PRIVATE Qt6::Core Qt6::Widgets Qt6::Concurrent)
target_include_directories(aidoctools PRIVATE include)

# 设置为窗口应用程序，去掉控制台输出
set_target_properties(aidoctools PROPERTIES
    WIN32_EXECUTABLE ON
    MACOSX_BUNDLE ON
)

# 添加静态链接选项，尽量减少依赖
if(MSVC)
    target_link_options(aidoctools PRIVATE /INCREMENTAL:NO /LTCG)
    if(CMAKE_BUILD_TYPE STREQUAL "Release")
        set_property(TARGET aidoctools PROPERTY MSVC_RUNTIME_LIBRARY "MultiThreaded")
    endif()
endif()

# 设置安装规则
install(TARGETS aidoctools DESTINATION bin)

# Qt部署工具配置
if(WIN32)
    # 使用windeployqt工具打包Qt依赖
    find_program(WINDEPLOYQT_EXECUTABLE windeployqt HINTS "${CMAKE_PREFIX_PATH}/bin")
    if(WINDEPLOYQT_EXECUTABLE)
        add_custom_command(TARGET aidoctools POST_BUILD
            COMMAND "${WINDEPLOYQT_EXECUTABLE}"
                --no-translations
                --no-system-d3d-compiler
                --no-opengl-sw
                --no-compiler-runtime
                --release
                --verbose 0
                "$<TARGET_FILE:aidoctools>"
            COMMENT "Running windeployqt..."
        )
    endif()
endif()
