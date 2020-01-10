set(CURRENT_TARGET "subtitle-renderer")

message(STATUS "\nConfiguring target ${CURRENT_TARGET}...")

CreateTarget(${CURRENT_TARGET} STATIC subtitle-renderer 17)

# Find Qt Framework
message(STATUS "Finding Qt...")
find_package(Qt5Core REQUIRED)
find_package(Qt5Gui REQUIRED)
message(STATUS "Qt version: ${Qt5Core_VERSION}")

# Qt deprecated warnings
add_definitions(-DQT_DEPRECATED_WARNINGS)
add_definitions(-DQT_DISABLE_DEPRECATED_BEFORE=0x060000)

# Qt: disable some features
add_definitions(-DQT_NO_FOREACH)

# find ImageMagick C++ library
pkg_check_modules(MAGICKPP REQUIRED Magick++)
message(STATUS "Magick++ library: ${IMAGICKPP_LIBRARIES}")
message(STATUS "Magick++ include directory: ${MAGICKPP_INCLUDE_DIRS}")

set(SUBTITLERENDERER_INCLUDE_DIR "${CURRENT_TARGET_DIR}/include")
set(SUBTITLERENDERER_INCLUDE_DIR "${CURRENT_TARGET_DIR}/include" PARENT_SCOPE)
message(STATUS "${CURRENT_TARGET} include directory: ${SUBTITLERENDERER_INCLUDE_DIR}")

target_include_directories(${CURRENT_TARGET} PRIVATE ${MAGICKPP_INCLUDE_DIRS})
target_include_directories(${CURRENT_TARGET} PRIVATE "${CURRENT_TARGET_DIR}/include/renderer")

target_link_libraries(${CURRENT_TARGET}
PUBLIC
    Qt5::Core
    Qt5::Gui
PRIVATE
    SubtitleParserInterface
    ${MAGICKPP_LIBRARIES}
    reprocxx
)

# update version file on changes
if (INCLUDE_GIT_TRACKING)
    add_dependencies(${CURRENT_TARGET} check_git_repository)
endif()

add_library(SubtitleRendererInterface INTERFACE)
target_include_directories(SubtitleRendererInterface INTERFACE "${SUBTITLERENDERER_INCLUDE_DIR}")
target_link_libraries(SubtitleRendererInterface INTERFACE ${CURRENT_TARGET})

message(STATUS "Configured target ${CURRENT_TARGET}.\n")
