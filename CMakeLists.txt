cmake_minimum_required(VERSION 3.8.0)
project(BezierLightLTC)

if (WIN32)
    set(GLFW3_DIR "GLFW3_DIR" CACHE PATH "")
    set(GLM_DIR "GLM_DIR" CACHE PATH "")
endif()

list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_LIST_DIR}/cmake")

if (UNIX AND NOT APPLE)
    set(LINUX TRUE)
endif()

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
if (UNIX)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g -Wall -pthread")
endif()

if ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "GNU")
    add_compile_options(-fdiagnostics-color=always)
elseif ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "Clang")
    add_compile_options(-fcolor-diagnostics)
elseif ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "AppleClang")
    add_compile_options(-fcolor-diagnostics)
endif ()

# ----------
# Packages
# ----------
find_package(OpenGL REQUIRED)
find_package(GLFW3 REQUIRED)
find_package(GLM REQUIRED)

# ----------
# Output paths
# ----------
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_RELEASE ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY_RELEASE ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY_RELEASE ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_DEBUG ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY_DEBUG ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY_DEBUG ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_DEBUG_POSTFIX "-debug")

# ----------
# OS specific settings
# ----------
set(COMMON_INCLUDE_DIRS ${OPENGL_INCLUDE_DIRS}
                        ${GLFW3_INCLUDE_DIRS}
                        ${GLM_INCLUDE_DIRS})
set(COMMON_LIBRARIES ${OPENGL_LIBRARIES}
                     ${GLFW3_LIBRARIES})

if (MSVC)
    add_definitions(-D_CRT_SECURE_NO_WARNINGS)
endif()

if (MINGW)
    set(COMMON_LIBRARIES ${COMMON_LIBRARIES} imm32 dwmapi)
endif()

if (APPLE)
    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -framework Cocoa -framework OpenGL -framework IOKit -framework CoreVideo")
    set(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} -framework Cocoa -framework OpenGL -framework IOKit -framework CoreVideo")
endif()

if (LINUX)
    find_package(PkgConfig REQUIRED)
    pkg_search_module(GLFW3 REQUIRED glfw3)
    set(COMMON_LIBRARIES ${COMMON_LIBRARIES} ${CMAKE_DL_LIBS} ${GLFW3_STATIC_LIBRARIES})
endif()

# ----------
# Traverse subdirectories
# ----------

file(GLOB SHADER_FILES "shaders/*.vert" "shaders/*.frag")
add_subdirectory(src)
