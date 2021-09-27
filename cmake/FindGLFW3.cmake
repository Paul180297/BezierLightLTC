include(FindPackageHandleStandardArgs)

set(GLFW3_DIR "" CACHE PATH "")

find_path(GLFW3_INCLUDE_DIR
        NAMES GLFW/glfw3.h
        PATHS
        /usr/include
        /usr/local/include
        /usr/local/Cellar/glfw3/include
        ${GLFW3_DIR}/include)

find_library(GLFW3_LIBRARY
        NAMES glfw3 glfw
        PATHS
        /usr/lib
        /usr/local/lib
        /usr/local/Cellar/glfw3/lib
        ${GLFW3_DIR}/lib)

find_package_handle_standard_args(
    GLFW3
    DEFAULT_MSG
    GLFW3_INCLUDE_DIR
    GLFW3_LIBRARY
)

mark_as_advanced(GLFW3_DIR GLFW3_INCLUDE_DIR GLFW3_LIBRARY)

if (GLFW3_FOUND)
    message(STATUS "GLFW3 include: ${GLFW3_INCLUDE_DIR}")
    message(STATUS "GLFW3 library: ${GLFW3_LIBRARY}")
    set(GLFW3_INCLUDE_DIRS "${GLFW3_INCLUDE_DIR}" CACHE PATH "")
    set(GLFW3_LIBRARIES "${GLFW3_LIBRARY}" CACHE FILEPATH "")
    mark_as_advanced(GLFW3_INCLUDE_DIR GLFW3_LIBRARY)
endif()
