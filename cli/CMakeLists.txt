set(CURRENT_TARGET "cli")

message(STATUS "\nConfiguring target ${CURRENT_TARGET}...")

CreateTarget(${CURRENT_TARGET} EXECUTABLE jimaku-renderer 17)

set(CLI_INCLUDE_DIR "${CURRENT_TARGET_DIR}")
set(CLI_INCLUDE_DIR "${CURRENT_TARGET_DIR}" PARENT_SCOPE)
message(STATUS "${CURRENT_TARGET} include directory: ${CLI_INCLUDE_DIR}")

target_link_libraries(${CURRENT_TARGET}
PRIVATE
    SubtitleParserInterface
    SubtitleRendererInterface
    ProjectConfigInterface
)

# update version file on changes
if (INCLUDE_GIT_TRACKING)
    add_dependencies(${CURRENT_TARGET} check_git_repository)
endif()

set_target_properties(${CURRENT_TARGET} PROPERTIES PREFIX "")
set_target_properties(${CURRENT_TARGET} PROPERTIES OUTPUT_NAME "jimaku-renderer")

message(STATUS "Configured target ${CURRENT_TARGET}.\n")
