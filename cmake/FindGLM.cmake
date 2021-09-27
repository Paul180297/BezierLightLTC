include(FindPackageHandleStandardArgs)

set(GLM_DIR "" CACHE PATH "")

find_path(GLM_INCLUDE_DIR
          NAMES glm/glm.hpp
          PATHS
          /usr/include
          /usr/local/include
          ${GLM_DIR}
          ${GLM_DIR}/include)

find_package_handle_standard_args(
    GLM
    DEFAULT_MSG
    GLM_INCLUDE_DIR
)

if (GLM_FOUND)
    message(STATUS "GLM include: ${GLM_INCLUDE_DIR}")
    set(GLM_INCLUDE_DIRS "${GLM_INCLUDE_DIR}" CACHE PATH "")
    mark_as_advanced(GLM_INCLUDE_DIR)
endif()
