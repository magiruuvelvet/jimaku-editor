set(CURRENT_TARGET "subtitle-parser")

message(STATUS "\nConfiguring target ${CURRENT_TARGET}...")

CreateTarget(${CURRENT_TARGET} STATIC subtitle-parser 17)

set(SUBTITLEPARSER_INCLUDE_DIR "${CURRENT_TARGET_DIR}/include")
set(SUBTITLEPARSER_INCLUDE_DIR "${CURRENT_TARGET_DIR}/include" PARENT_SCOPE)
message(STATUS "${CURRENT_TARGET} include directory: ${SUBTITLEPARSER_INCLUDE_DIR}")

target_include_directories(${CURRENT_TARGET} PRIVATE "${CURRENT_TARGET_DIR}/include/srtparser")

# update version file on changes
if (INCLUDE_GIT_TRACKING)
    add_dependencies(${CURRENT_TARGET} check_git_repository)
endif()

add_library(SubtitleParserInterface INTERFACE)
target_include_directories(SubtitleParserInterface INTERFACE "${SUBTITLEPARSER_INCLUDE_DIR}")
target_link_libraries(SubtitleParserInterface INTERFACE ${CURRENT_TARGET})

message(STATUS "Configured target ${CURRENT_TARGET}.\n")
