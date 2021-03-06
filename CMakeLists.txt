cmake_minimum_required(VERSION 3.14)
set(CMAKE_MODULE_PATH "${CMAKE_MODULE_PATH}" "${CMAKE_CURRENT_LIST_DIR}/cmake/modules")

include(ProjectSetup)

# 字幕エーディタ (Subtitle Editor)
project(jimaku-editor CXX C)

DetectAdditionalDebugOptions()

# Setup project version config
option(INCLUDE_GIT_TRACKING "Include git version tracking" ON)
set(VERSION_TEMPLATE_FILE "${CMAKE_SOURCE_DIR}/config/version_private.hpp.in")
set(VERSION_TARGET_FILE "${CMAKE_BINARY_DIR}/include/config/version_private.hpp")
file(MAKE_DIRECTORY "${CMAKE_BINARY_DIR}/include/config")
include_directories("${CMAKE_BINARY_DIR}/include")

# Include git version tracking - dynamically updated on rebuild
# note to developers: the version.cpp file isn't recreated when
# deleted and there were no repo changes, delete the git-state
# file to force recreation of the version.cpp file
if (INCLUDE_GIT_TRACKING)
    message(STATUS "git version tracking enabled.")
    set(PRE_CONFIGURE_FILE "${VERSION_TEMPLATE_FILE}")
    set(POST_CONFIGURE_FILE "${VERSION_TARGET_FILE}")
    include(GitWatcher)

# Only write the program version without git - static, no longer updated (release builds)
else()
    set(GIT_RETRIEVED_STATE "false")
    set(GIT_IS_DIRTY "false")
    set(GIT_HEAD_SHA1 "")
    configure_file("${VERSION_TEMPLATE_FILE}" "${VERSION_TARGET_FILE}" @ONLY)
endif()

# pkg-config
find_package(PkgConfig REQUIRED)
if (NOT PKG_CONFIG_FOUND)
    message(FATAL_ERROR "pkg-config executable not found on your system!")
endif()

# Dependencies
message(STATUS "\n>>> START configure deps")
add_subdirectory(libs)
message(STATUS ">>> END configure deps\n")

# コンフィグレーション
add_subdirectory(config)

# プロジェクト・モチュール
add_subdirectory(subtitle-parser)
add_subdirectory(subtitle-renderer)
add_subdirectory(pgs-encoder)

# User Interface
add_subdirectory(cli)

# ユニットテスト
option(ENABLE_TESTS "Build the unit tests" ON)
if (ENABLE_TESTS)
    message(STATUS "Unit tests enabled.")
    add_subdirectory(tests)
endif()
