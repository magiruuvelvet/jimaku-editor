include(SetCppStandard)

macro(CreateTarget CMakeTargetName Type OutputName CppVer)
    # grep files from current directory
    if (NOT ${CppVer} EQUAL 0)
        file(GLOB_RECURSE SourceList
            "*.cpp"
            "*.hpp"
        )
    else()
        file(GLOB_RECURSE SourceList
            "*.c"
            "*.h"
        )
    endif()

    # create target
    if (${Type} STREQUAL "EXECUTABLE")
        add_executable(${CMakeTargetName} ${SourceList})
    elseif(${Type} STREQUAL "SHARED")
        add_library(${CMakeTargetName} SHARED ${SourceList})
    elseif(${Type} STREQUAL "STATIC")
        add_library(${CMakeTargetName} STATIC ${SourceList})
    else()
        message(FATAL_ERROR "CreateTarget: unsupported type: ${Type}")
    endif()

    # set output name
    set_target_properties(${CMakeTargetName} PROPERTIES PREFIX "")
    set_target_properties(${CMakeTargetName} PROPERTIES OUTPUT_NAME "${OutputName}")

    if (NOT ${CppVer} EQUAL 0)
        # sets the required C++ version on the target
        SetCppStandard(${CMakeTargetName} ${CppVer})
        set_target_properties(${CMakeTargetName} PROPERTIES LINKER_LANGUAGE CXX)
    else()
        SetCStandard(${CMakeTargetName} 11)
        set_target_properties(${CMakeTargetName} PROPERTIES LINKER_LANGUAGE C)
    endif()

    # add current directory to include paths of the target
    target_include_directories(${CMakeTargetName} PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}")

    # export variable with the current target source directory
    set(CURRENT_TARGET_DIR "${CMAKE_CURRENT_SOURCE_DIR}")
endmacro()
