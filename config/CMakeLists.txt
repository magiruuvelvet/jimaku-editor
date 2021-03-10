set(CURRENT_TARGET "config")

CreateTarget(${CURRENT_TARGET} STATIC project-config C++ 17)

set(PROJECTCONFIG_INCLUDE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/include")
set(PROJECTCONFIG_INCLUDE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/include" PARENT_SCOPE)
message(STATUS "${CURRENT_TARGET} include directory: ${PROJECTCONFIG_INCLUDE_DIR}")

target_include_directories(${CURRENT_TARGET} PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/include/config")

# update version file on changes
if (INCLUDE_GIT_TRACKING)
    add_dependencies(${CURRENT_TARGET} check_git_repository)
endif()

add_library(ProjectConfigInterface INTERFACE)
target_include_directories(ProjectConfigInterface INTERFACE "${PROJECTCONFIG_INCLUDE_DIR}")
target_link_libraries(ProjectConfigInterface INTERFACE ${CURRENT_TARGET})
