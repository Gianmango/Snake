cmake_minimum_required(VERSION 3.22)
project(Snake LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

list(APPEND CMAKE_PREFIX_PATH "C:/Users/ermeg/AppData/Local/Programs/SFML")

find_package(SFML 3 REQUIRED COMPONENTS Graphics)

add_executable(Snake src/main.cpp)

target_link_libraries(Snake PRIVATE SFML::Graphics)

if(WIN32)
    add_custom_command(
        TARGET Snake
        POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
            $<TARGET_RUNTIME_DLLS:Snake>
            $<TARGET_FILE_DIR:Snake>
        COMMAND_EXPAND_LISTS
    )
endif()