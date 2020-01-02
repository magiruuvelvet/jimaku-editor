set(CURRENT_TARGET "SubtitleRenderer")

message(STATUS "\nConfiguring target ${CURRENT_TARGET}...")

CreateTarget(${CURRENT_TARGET} STATIC SubtitleRenderer 17)

set(SUBTITLERENDERER_INCLUDE_DIR "${CURRENT_TARGET_DIR}")
set(SUBTITLERENDERER_INCLUDE_DIR "${CURRENT_TARGET_DIR}" PARENT_SCOPE)
message(STATUS "${CURRENT_TARGET} include directory: ${SUBTITLERENDERER_INCLUDE_DIR}")

add_library(SubtitleRendererInterface INTERFACE)
target_include_directories(SubtitleRendererInterface INTERFACE "${SUBTITLERENDERER_INCLUDE_DIR}")
target_link_libraries(SubtitleRendererInterface INTERFACE SubtitleRenderer)

message(STATUS "Configured target ${CURRENT_TARGET}.\n")
