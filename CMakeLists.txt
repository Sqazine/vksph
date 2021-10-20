cmake_minimum_required(VERSION 3.10)
set(VCPKG_ROOT "$ENV{VCPKG_ROOT}")
set(VCPKG_TARGET_TRIPLET "$ENV{VCPKG_DEFAULT_TRIPLET}")
set(CMAKE_TOOLCHAIN_FILE "${VCPKG_ROOT}/scripts/buildsystems/vcpkg.cmake")

project(vksph LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)

set(VULKAN_ROOT "$ENV{VULKAN_SDK}" CACHE STRING "")
message(STATUS ${VULKAN_ROOT})

file(GLOB SRC "*.h" "*.cpp")

find_package(SDL2 CONFIG REQUIRED)
find_package(glm CONFIG REQUIRED)
add_executable(vksph ${SRC})
target_include_directories(vksph PRIVATE "${VULKAN_ROOT}/Include")
target_link_libraries(vksph PRIVATE SDL2::SDL2 glm::glm "${VULKAN_ROOT}/Lib/vulkan-1.lib")
target_compile_definitions(vksph PRIVATE SHADER_DIR="${CMAKE_SOURCE_DIR}/shaders/" )