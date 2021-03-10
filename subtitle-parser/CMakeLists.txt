set(CURRENT_TARGET "subtitle-parser")

CreateTarget(${CURRENT_TARGET} STATIC subtitle-parser C++ 17)

set(SUBTITLEPARSER_INCLUDE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/include")
set(SUBTITLEPARSER_INCLUDE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/include" PARENT_SCOPE)
message(STATUS "${CURRENT_TARGET} include directory: ${SUBTITLEPARSER_INCLUDE_DIR}")

target_include_directories(${CURRENT_TARGET} PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/include/srtparser")

# update version file on changes
if (INCLUDE_GIT_TRACKING)
    add_dependencies(${CURRENT_TARGET} check_git_repository)
endif()

add_library(SubtitleParserInterface INTERFACE)
target_include_directories(SubtitleParserInterface INTERFACE "${SUBTITLEPARSER_INCLUDE_DIR}")
target_link_libraries(SubtitleParserInterface INTERFACE ${CURRENT_TARGET})
