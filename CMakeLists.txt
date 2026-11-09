include(./.env.cmake OPTIONAL RESULT_VARIABLE LOCAL_ENV)
message(STATUS "Local .env.cmake: ${LOCAL_ENV}")

cmake_minimum_required(VERSION 3.21)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/Run)

project(MyProject VERSION 1.0)

# ===== Vulkan ==================================================== #

find_package(Vulkan REQUIRED)

# ===== GLFW ====================================================== #

find_package(glfw3 REQUIRED)

# ===== My utils ================================================== #

file(GLOB_RECURSE UTILS_SOURCES ${UTILS_PATH}/src/*.cpp)

# Remove TIFF from sources
list(REMOVE_ITEM UTILS_SOURCES ${UTILS_PATH}/src/loadTif.cpp)
list(REMOVE_ITEM UTILS_SOURCES ${UTILS_PATH}/src/saveTif.cpp)

add_library(Utils STATIC ${UTILS_SOURCES})

target_include_directories(Utils PUBLIC ${UTILS_PATH}/include)

# ===== Main ====================================================== #

file(GLOB_RECURSE SOURCES ${PROJECT_SOURCE_DIR}/src/*.cpp)

add_executable(${PROJECT_NAME} ${SOURCES})

target_compile_options(${PROJECT_NAME} PRIVATE -Wall)
target_compile_options(${PROJECT_NAME} PRIVATE -Wextra)

target_precompile_headers(${PROJECT_NAME} PRIVATE ${PROJECT_SOURCE_DIR}/src/pch.hpp)

target_include_directories(${PROJECT_NAME} SYSTEM PRIVATE
  ${VULKAN_INCLUDE_DIRS}
  ${GLFW_INCLUDE_DIRS}
  ${TOL_PATH}/include
  ${GLM_PATH}/include
  ${STB_PATH}/include
)

target_link_libraries(${PROJECT_NAME} PRIVATE
  Vulkan::Vulkan
  glfw
  Utils
)

# ===== Build Shaders ============================================= #

# Find all vertex and fragment sources within shaders directory
# taken from VBlancos vulkan tutorial
# https://github.com/vblanco20-1/vulkan-guide/blob/all-chapters/CMakeLists.txt
find_program(GLSL_VALIDATOR glslangValidator HINTS
  ${VULKAN_GLSLANG_VALIDATOR_EXECUTABLE}
  /usr/bin
  /usr/local/bin
  ${VULKAN_SDK_PATH}/Bin
  ${VULKAN_SDK_PATH}/Bin32
  $ENV{VULKAN_SDK}/Bin/
  $ENV{VULKAN_SDK}/Bin32/
)

# get all .vert and .frag files in shaders directory
file(GLOB_RECURSE GLSL_SOURCE_FILES
  ${PROJECT_SOURCE_DIR}/src/shaders/*.frag
  ${PROJECT_SOURCE_DIR}/src/shaders/*.vert
)

message(STATUS "Compiling Shaders")
foreach(GLSL_FILE ${GLSL_SOURCE_FILES})
  message2("Compiling: [${GLSL_FILE}]" ${ColorBlue})
  cmake_path(GET GLSL_FILE FILENAME GLSL_FILE_NAME)
  set(SPIRV_HOME "${PROJECT_SOURCE_DIR}/Build/spv")
  set(SPIRV "${SPIRV_HOME}/${GLSL_FILE_NAME}.spv")
  add_custom_command(
    OUTPUT ${SPIRV}
    COMMAND ${CMAKE_COMMAND} -E make_directory ${SPIRV_HOME}
    COMMAND ${GLSL_VALIDATOR} -V ${GLSL_FILE} -o ${SPIRV}
    DEPENDS ${GLSL_FILE})
  list(APPEND SPIRV_BINARY_FILES ${SPIRV})
endforeach(GLSL_FILE)

add_custom_target(
  Shaders
  DEPENDS ${SPIRV_BINARY_FILES}
)

add_dependencies(${PROJECT_NAME} Shaders)

