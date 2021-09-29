# ----------------------------------
# Define ImGui interface
# ----------------------------------
set(IMGUI_INTERFACE imgui)

add_definitions(-DIMGUI_IMPL_OPENGL_LOADER_GLAD2)
add_library(${IMGUI_INTERFACE} INTERFACE)
file(GLOB
    IMGUI_FILES
    ${CMAKE_CURRENT_SOURCE_DIR}/imgui/*.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/imgui/*.h
)
target_sources(${IMGUI_INTERFACE} INTERFACE ${IMGUI_FILES})
source_group("Imgui" FILES ${IMGUI_FILES})

# ----------------------------------
# Define main build target
# ----------------------------------
set(BUILD_TARGET bezier_ltc)
file(GLOB
    SOURCE_FILES
    ${CMAKE_CURRENT_SOURCE_DIR}/*.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/*.h
)

add_executable(${BUILD_TARGET})

target_sources(
    ${BUILD_TARGET}
    PRIVATE
    ${SOURCE_FILES}
    ${SHADER_FILES}
)

source_group("Source Files" FILES ${SOURCE_FILES})
source_group("Shader Files" FILES ${SHADER_FILES})

include_directories(
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/stb
    ${CMAKE_CURRENT_SOURCE_DIR}/tinyobjloader
    ${CMAKE_CURRENT_SOURCE_DIR}/imgui
    ${COMMON_INCLUDE_DIRS}
)

target_link_libraries(
    ${BUILD_TARGET}
    PRIVATE
    ${COMMON_LIBRARIES}
    ${IMGUI_INTERFACE}
)

if (MSVC)
    target_compile_options(${BUILD_TARGET} PUBLIC "/Zi")
    set_property(TARGET ${BUILD_TARGET} APPEND PROPERTY LINK_FLAGS "/ignore:4099 /DEBUG /PROFILE")
endif()
