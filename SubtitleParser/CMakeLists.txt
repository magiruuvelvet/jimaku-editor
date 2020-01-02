set(CURRENT_TARGET "SubtitleParser")

message(STATUS "\nConfiguring target ${CURRENT_TARGET}...")

CreateTarget(${CURRENT_TARGET} STATIC SubtitleParser 17)

set(SUBTITLEPARSER_INCLUDE_DIR "${CURRENT_TARGET_DIR}")
set(SUBTITLEPARSER_INCLUDE_DIR "${CURRENT_TARGET_DIR}" PARENT_SCOPE)
message(STATUS "${CURRENT_TARGET} include directory: ${SUBTITLEPARSER_INCLUDE_DIR}")

add_library(SubtitleParserInterface INTERFACE)
target_include_directories(SubtitleParserInterface INTERFACE "${SUBTITLEPARSER_INCLUDE_DIR}")
target_link_libraries(SubtitleParserInterface INTERFACE SubtitleParser)

message(STATUS "Configured target ${CURRENT_TARGET}.\n")
