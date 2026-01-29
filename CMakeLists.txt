cmake_minimum_required(VERSION 3.24)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

if (${CMAKE_SYSTEM_PROCESSOR} STREQUAL "x86_64")
    add_compile_options(-mf16c -mavx -mfma -msse4)
elseif (${CMAKE_SYSTEM_PROCESSOR} STREQUAL "aarch64")
endif()

file(MAKE_DIRECTORY ${CMAKE_SOURCE_DIR}/build)
file(MAKE_DIRECTORY ${CMAKE_SOURCE_DIR}/shaders/vulkan)

set(CMAKE_CONFIGURATION_TYPES Debug;Release)
set(CMAKE_DEBUG_POSTFIX -dev)
set(CMAKE_POSITION_INDEPENDENT_CODE ON)

option(MEMORY_TRACKING "Enables address sanitizer." OFF)

project(nxc-framework LANGUAGES CXX)

find_package(Vulkan REQUIRED)

if (${CMAKE_SYSTEM_NAME} STREQUAL "Linux")
    add_compile_definitions(NXC_TARGET_DESKTOP=1)
elseif(${CMAKE_SYSTEM_NAME} STREQUAL "Windows")
    add_compile_definitions(NXC_TARGET_DESKTOP=1)
elseif(${CMAKE_SYSTEM_NAME} STREQUAL "Android")
endif()

add_library(${PROJECT_NAME} STATIC)

target_include_directories(${PROJECT_NAME} PRIVATE
    ${CMAKE_SOURCE_DIR}/include
)

message(STATUS "MEMORY_TRACKING: ${MEMORY_TRACKING}")

target_sources(${PROJECT_NAME} PRIVATE
    "src-lib/err/gpgpu/vulkan/logical_device_creation_failed.cpp"
    "src-lib/err/gpgpu/vulkan/no_compatible_device_found.cpp"
    "src-lib/err/gpgpu/vulkan/system_connection_failure_sdl.cpp"
    "src-lib/err/gpgpu/vulkan/system_connection_failure.cpp"
    "src-lib/err/vid/window_append.cpp"
    "src-lib/err/vid/window_name_in_use.cpp"
    "src-lib/subsystems/logger/logger.cpp"
    "src-lib/subsystems/logger/message.cpp"
    "src-lib/subsystems/gpgpu/vulkan/dynamic_functions.cpp"
    "src-lib/subsystems/gpgpu/base/root.cpp"
    "src-lib/subsystems/gpgpu/vulkan/dvc/dvc_requirements.cpp"
    "src-lib/subsystems/gpgpu/vulkan/dvc/dctor.cpp"
    "src-lib/subsystems/gpgpu/vulkan/dvc/dvc_info.cpp"
    "src-lib/subsystems/gpgpu/vulkan/dvc/dvc_processor_stage_resources.cpp"
    "src-lib/subsystems/gpgpu/vulkan/mem_manager/allocation.cpp"
    "src-lib/subsystems/gpgpu/vulkan/mem_manager/dctor.cpp"
    "src-lib/subsystems/gpgpu/vulkan/mem_manager/deallocation.cpp"
    "src-lib/subsystems/gpgpu/vulkan/root/root.cpp"
    "src-lib/subsystems/gpgpu/vulkan/root/wnd_ext_handling.cpp"
    "src-lib/subsystems/gpgpu/vulkan/root/handling/presentation.cpp"
    "src-lib/subsystems/gpgpu/vulkan/root/handling/update_ui.cpp"
    "src-lib/subsystems/gpgpu/vulkan/root/handling/proc_stage.cpp"
    "src-lib/subsystems/gpgpu/vulkan/proc_stage_resources/stage_ui_colorbox.cpp"
    "src-lib/subsystems/gpgpu/vulkan/processor_stages/render_ui_colorbox.cpp"
    "src-lib/subsystems/time/time_root.cpp"
    "src-lib/subsystems/time/registry.cpp"
    "src-lib/subsystems/time/point.cpp"
    "src-lib/subsystems/vid/vidroot.cpp"
    "src-lib/subsystems/vid/windows.cpp"
    "src-lib/subsystems/subsystems.cpp"
    "src-lib/utils/file_utils.cpp"
    "src-lib/main.cpp"
)
target_link_libraries(${PROJECT_NAME} PRIVATE
    "SDL3"
    "vulkan"
)
target_compile_definitions(${PROJECT_NAME} PRIVATE
    NXC_APP_NAME="./${PROJECT_NAME}"
)

if (${MEMORY_TRACKING})
    target_link_options(${PROJECT_NAME} PRIVATE
        -fsanitize=address
        -static-libasan
    )
    target_compile_options(${PROJECT_NAME} PRIVATE
        -fsanitize=address -g
    ) 
endif()

set(VULKAN_SHADER_SOURCES 
    "ui_colorbox.vert"
    "ui_colorbox.frag"
)

message(STATUS "Compiling Vulkan shaders")
foreach(SHADER_ENTRY ${VULKAN_SHADER_SOURCES})
    execute_process(
        COMMAND glslangValidator --target-env vulkan1.1 "${CMAKE_SOURCE_DIR}/src-shaders/vulkan/${SHADER_ENTRY}" -o "${SHADER_ENTRY}.spv"
        WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}/shaders/vulkan
    )
endforeach()

# clear && cmake -B build -G "Ninja Multi-Config" -DCMAKE_SYSTEM_PROCESSOR=x86_64 && ninja -C build -f build-Debug.ninja && ninja -C build -f build-Release.ninja