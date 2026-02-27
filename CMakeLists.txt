cmake_minimum_required(VERSION 3.24)

set(CMAKE_CXX_STANDARD 26)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

file(MAKE_DIRECTORY ${CMAKE_SOURCE_DIR}/shaders/vulkan)
file(MAKE_DIRECTORY ${CMAKE_SOURCE_DIR}/tools)
file(MAKE_DIRECTORY ${CMAKE_SOURCE_DIR}/bin)

set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/bin)

set(CMAKE_CONFIGURATION_TYPES Debug;Release)
set(CMAKE_POSITION_INDEPENDENT_CODE ON)

option(MEMORY_TRACKING "Enables address sanitizer." OFF)

project(redfx_engine LANGUAGES CXX VERSION 0.0.1)
string(TOLOWER "${CMAKE_SYSTEM_NAME}-${CMAKE_SYSTEM_PROCESSOR}" PLATFORM_SUFFIX)
set(CMAKE_DEBUG_POSTFIX "-${PROJECT_VERSION}-dev-${PLATFORM_SUFFIX}")
set(CMAKE_RELEASE_POSTFIX "-${PROJECT_VERSION}-${PLATFORM_SUFFIX}")
 
find_package(Vulkan REQUIRED)

add_library(${PROJECT_NAME} STATIC)

target_include_directories(${PROJECT_NAME} PRIVATE
    ${CMAKE_SOURCE_DIR}/include
)

message(STATUS "MEMORY_TRACKING: ${MEMORY_TRACKING}")
message(STATUS "TARGET ARCHITECTURE: ${CMAKE_SYSTEM_PROCESSOR}")
message(STATUS "PROJECT VERSION: ${CMAKE_PROJECT_VERSION}")

set(SRC_LIB_GPGPU
    "src-lib/err/gpgpu/vulkan/logical_device_creation_failed.cpp"
    "src-lib/err/gpgpu/vulkan/no_compatible_device_found.cpp"
    "src-lib/err/gpgpu/vulkan/system_connection_failure_sdl.cpp"
    "src-lib/err/gpgpu/vulkan/system_connection_failure.cpp"
    "src-lib/subsystems/gpgpu/vulkan/dynamic_functions.cpp"
    "src-lib/subsystems/gpgpu/base/root.cpp"
    "src-lib/subsystems/gpgpu/vulkan/dvc/dvc_requirements.cpp"
    "src-lib/subsystems/gpgpu/vulkan/dvc/dctor.cpp"
    "src-lib/subsystems/gpgpu/vulkan/dvc/dvc_info.cpp"
    "src-lib/subsystems/gpgpu/vulkan/dvc/dvc_processor_stage_resources.cpp"
    "src-lib/subsystems/gpgpu/vulkan/dvc/transfer_handling.cpp"
    "src-lib/subsystems/gpgpu/vulkan/mem_manager/allocation.cpp"
    "src-lib/subsystems/gpgpu/vulkan/mem_manager/dctor.cpp"
    "src-lib/subsystems/gpgpu/vulkan/mem_manager/deallocation.cpp"
    "src-lib/subsystems/gpgpu/vulkan/root/root.cpp"
    "src-lib/subsystems/gpgpu/vulkan/root/wnd_ext_handling.cpp"
    "src-lib/subsystems/gpgpu/vulkan/root/handling/presentation.cpp"
    "src-lib/subsystems/gpgpu/vulkan/root/handling/update_ui.cpp"
    "src-lib/subsystems/gpgpu/vulkan/root/handling/proc_stage.cpp"
    "src-lib/subsystems/gpgpu/vulkan/processor_stages/renderui_cull_elements.cpp"
    "src-lib/subsystems/gpgpu/vulkan/processor_stages/render_ui.cpp"
    "src-lib/subsystems/gpgpu/vulkan/proc_stage_resources/renderui_bitmapbox.cpp"
    "src-lib/subsystems/gpgpu/vulkan/proc_stage_resources/renderui_imagebox.cpp"
    "src-lib/subsystems/gpgpu/vulkan/proc_stage_resources/renderui_colorbox.cpp"
    "src-lib/subsystems/gpgpu/vulkan/proc_stage_resources/renderui_textbox.cpp"
    "src-lib/subsystems/gpgpu/vulkan/proc_stage_resources/renderui_general.cpp"
    "src-lib/subsystems/gpgpu/vulkan/proc_stage_resources/renderui_cull_elements.cpp"
)
set(SRC_LIB_VID
    "src-lib/err/vid/window_append.cpp"
    "src-lib/err/vid/window_name_in_use.cpp"
    "src-lib/subsystems/vid/vidroot.cpp"
    "src-lib/subsystems/vid/windows.cpp"
)
set(SRC_LIB_UI
    "src-lib/ui/analyze_pages.cpp"
    "src-lib/ui/analyze_windows.cpp"
    "src-lib/ui/main.cpp"
)
set(SRC_LIB_RESOURCE_MANAGER
    "src-lib/subsystems/resources_manager/root.cpp"
    "src-lib/subsystems/resources_manager/resource_data_redfxfont.cpp"
    "src-lib/subsystems/resources_manager/resource_data_redfxui.cpp"
    "src-lib/subsystems/resources_manager/resource_data_image.cpp"
    "src-lib/subsystems/resources_manager/asynchronous_controller.cpp"
)

target_sources(${PROJECT_NAME} PRIVATE
    ${SRC_LIB_GPGPU}
    ${SRC_LIB_VID}
    ${SRC_LIB_UI}
    ${SRC_LIB_RESOURCE_MANAGER}
    "src-lib/subsystems/logger/logger.cpp"
    "src-lib/subsystems/logger/message.cpp"
    "src-lib/subsystems/time/time_root.cpp"
    "src-lib/subsystems/time/registry.cpp"
    "src-lib/subsystems/time/point.cpp"
    "src-lib/subsystems/subsystems.cpp"
    "src-lib/utils/file_utils.cpp"
    "src-lib/stb.cpp"
    "src-lib/ui_class.cpp"
    "src-lib/main.cpp"
)
target_link_libraries(${PROJECT_NAME} PRIVATE
    "SDL3"
    "vulkan"
    "tinyxml2"
    "ttf2mesh"
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

file(GLOB_RECURSE VULKAN_SHADER_SOURCES "${CMAKE_SOURCE_DIR}/src-shaders/vulkan/*.vert" "${CMAKE_SOURCE_DIR}/src-shaders/vulkan/*.frag")

message(STATUS "Compiling Vulkan shaders")
foreach(SHADER_ENTRY ${VULKAN_SHADER_SOURCES})
    cmake_path(GET SHADER_ENTRY FILENAME FILE_NAME)
    execute_process(
        COMMAND glslangValidator --target-env vulkan1.1 "${SHADER_ENTRY}" -o "${FILE_NAME}.spv"
        WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}/shaders/vulkan
    )
endforeach()

# clear && cmake -B build -G "Ninja Multi-Config" -DCMAKE_SYSTEM_PROCESSOR=x86_64 && ninja -C build -f build-Debug.ninja && ninja -C build -f build-Release.ninja

# ttf2mesh git https://github.com/fetisov/ttf2mesh.git