cmake_minimum_required(VERSION 3.24)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

if (${CMAKE_SYSTEM_PROCESSOR} STREQUAL "x86_64")
    add_compile_options(-mf16c -mavx -mfma -msse4)
elseif (${CMAKE_SYSTEM_PROCESSOR} STREQUAL "aarch64")
endif()

file(MAKE_DIRECTORY ${CMAKE_SOURCE_DIR}/build)
file(MAKE_DIRECTORY ${CMAKE_SOURCE_DIR}/build_output)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/build_output)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/build_output)

set(CMAKE_CONFIGURATION_TYPES Debug;Release)
set(CMAKE_DEBUG_POSTFIX -dev)

option(MEMORY_TRACKING "Enables address sanitizer." OFF)

project(app LANGUAGES CXX)

find_package(Vulkan REQUIRED)

if (${CMAKE_SYSTEM_NAME} STREQUAL "Linux")
    add_compile_definitions(NXC_TARGET_DESKTOP=1)
elseif(${CMAKE_SYSTEM_NAME} STREQUAL "Windows")
    add_compile_definitions(NXC_TARGET_DESKTOP=1)
elseif(${CMAKE_SYSTEM_NAME} STREQUAL "Android")
endif()

add_library(${PROJECT_NAME} STATIC)

message(STATUS "MEMORY_TRACKING: ${MEMORY_TRACKING}")

target_sources(${PROJECT_NAME} PRIVATE
    "app_root.cpp"
)
target_link_libraries(${PROJECT_NAME} PRIVATE
    "SDL3"
    "vulkan"
    "nxc-framework"
)

if (${MEMORY_TRACKING})
    target_link_options(${PROJECT_NAME} PRIVATE
        -fsanitize=address -static-libasan
    )
    target_compile_options(${PROJECT_NAME} PRIVATE
        -fsanitize=address -g
    )
endif()