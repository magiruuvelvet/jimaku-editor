set(CURRENT_TARGET "pgs-encoder")

message(STATUS "\nConfiguring target ${CURRENT_TARGET}...")

CreateTarget(${CURRENT_TARGET} EXECUTABLE pgs-encoder C 11)

# find libpng
pkg_check_modules(LIBPNG REQUIRED libpng)
message(STATUS "libpng library: ${LIBPNG_LIBRARIES}")
message(STATUS "libpng include directory: ${LIBPNG_INCLUDE_DIRS}")

set(PGSENCODER_INCLUDE_DIR "${CURRENT_TARGET_DIR}")
set(PGSENCODER_INCLUDE_DIR "${CURRENT_TARGET_DIR}" PARENT_SCOPE)
message(STATUS "${CURRENT_TARGET} include directory: ${PGSENCODER_INCLUDE_DIR}")

target_include_directories(${CURRENT_TARGET} PRIVATE ${LIBPNG_INCLUDE_DIRS})

target_link_libraries(${CURRENT_TARGET}
PUBLIC
    m
PRIVATE
    ${LIBPNG_LIBRARIES}
)

# update version file on changes
if (INCLUDE_GIT_TRACKING)
    add_dependencies(${CURRENT_TARGET} check_git_repository)
endif()

set_target_properties(${CURRENT_TARGET} PROPERTIES PREFIX "")
set_target_properties(${CURRENT_TARGET} PROPERTIES OUTPUT_NAME "pgssup")

message(STATUS "Configured target ${CURRENT_TARGET}.\n")
