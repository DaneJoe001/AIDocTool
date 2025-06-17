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

add_executable(aidoctools ${SOURCES} ${RESOURCES})

target_compile_definitions(aidoctools PRIVATE RESOURCE_DIR="${RESOURCE_DIR}")
target_link_libraries(aidoctools PRIVATE Qt6::Core Qt6::Widgets Qt6::Concurrent)
target_include_directories(aidoctools PRIVATE include)

#设置qDebug在终端输出
set_target_properties(aidoctools PROPERTIES
    WIN32_EXECUTABLE OFF
    MACOSX_BUNDLE ON
)

# 添加测试程序
add_executable(test_filter_rules "source/test_filter_rules.cpp" "source/filefilterutil.cpp")
target_include_directories(test_filter_rules PRIVATE include)
target_link_libraries(test_filter_rules PRIVATE Qt6::Core)
set_target_properties(test_filter_rules PROPERTIES AUTOMOC ON)

# 添加批量文件重命名工具测试程序
add_executable(test_batch_rename "source/test_batch_rename.cpp" "source/batchrenamedialog.cpp")
target_include_directories(test_batch_rename PRIVATE include)
target_link_libraries(test_batch_rename PRIVATE Qt6::Core Qt6::Widgets)
set_target_properties(test_batch_rename PROPERTIES AUTOMOC ON)

# 添加代码统计工具测试程序
add_executable(test_code_stats "source/test_code_stats.cpp" "source/codestatsdialog.cpp")
target_include_directories(test_code_stats PRIVATE include)
target_link_libraries(test_code_stats PRIVATE Qt6::Core Qt6::Widgets Qt6::Concurrent)
set_target_properties(test_code_stats PROPERTIES AUTOMOC ON)

# 添加文档生成工具测试程序
add_executable(test_doc_generator "source/test_doc_generator.cpp" "source/docgeneratordialog.cpp")
target_include_directories(test_doc_generator PRIVATE include)
target_link_libraries(test_doc_generator PRIVATE Qt6::Core Qt6::Widgets Qt6::Concurrent)
set_target_properties(test_doc_generator PROPERTIES AUTOMOC ON)
