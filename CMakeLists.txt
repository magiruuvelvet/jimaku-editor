cmake_minimum_required(VERSION 3.14)

# 字幕エーディタ (Subtitle Editor)
project(jimaku-editor)

message(STATUS "Building on: ${CMAKE_SYSTEM_NAME} ${CMAKE_SYSTEM_VERSION}")

# プロジェクト設定
set(CMAKE_CONFIGURATION_TYPES Debug Release RelWithDebInfo)

# Use project folders in IDE's
set_property(GLOBAL PROPERTY USE_FOLDERS ON)

# Meta and Helper Modules
set(CMAKE_MODULE_PATH "${CMAKE_MODULE_PATH}"
    "${PROJECT_SOURCE_DIR}/cmake/modules"
    "${PROJECT_SOURCE_DIR}/cmake/libs")

# Set C++ standard macro
include(SetCppStandard)

# Set C standard macro
include(SetCStandard)

# Target creation macro
include(CreateTarget)

# Folder where generated files go
set(GENERATED_DIR ${CMAKE_CURRENT_BINARY_DIR}/Generated)
file(MAKE_DIRECTORY "${GENERATED_DIR}")
include_directories(${GENERATED_DIR})

# Handle configuration types
set(CMAKE_BUILD_TYPE "Debug" CACHE STRING "Set the CMake configuration type to use")
message(STATUS "Using configuration type: ${CMAKE_BUILD_TYPE}")
set(CMAKE_CONFIGURATION_TYPES ${CMAKE_BUILD_TYPE})

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

# Position Independent Code
set(CMAKE_POSITION_INDEPENDENT_CODE ON)

# Shared Libraries Configuration
set(CMAKE_INSTALL_RPATH "${ORIGIN}")
set(CMAKE_BUILD_WITH_INSTALL_RPATH TRUE)

# Enable (strict) C++17 features
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Release mode flags
set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -O2 -DNDEBUG")
set(CMAKE_C_FLAGS_RELEASE "${CMAKE_C_FLAGS_RELEASE} -O2 -DNDEBUG")

set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -rpath -fPIC")
set(CMAKE_C_FLAGS_RELEASE "${CMAKE_C_FLAGS_RELEASE} -rpath -fPIC")

set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -Wno-unused-command-line-argument")
set(CMAKE_C_FLAGS_RELEASE "${CMAKE_C_FLAGS_RELEASE} -Wno-unused-command-line-argument")

# Generic flags
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wno-unknown-pragmas -Wno-switch")
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wno-unknown-pragmas -Wno-switch")

# Debug mode flags
if ("${CMAKE_BUILD_TYPE}" STREQUAL "Debug")
    message(STATUS "Building with development configuration.")
    add_definitions(-DDEBUG_BUILD)

    if ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "Clang")
        message(STATUS "LLVM/clang detected. Enabling LLDB debugger tuning...")
        add_definitions("-glldb")
    endif()
endif()

# Set target destination for built targets
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib-static)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

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
