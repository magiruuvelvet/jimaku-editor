# 3rd party dependencies


###
### lib: reproc
###

message(STATUS "configure reproc...")
set(REPROCXX ON)
set(REPROC_TEST OFF)
set(REPROC_EXAMPLES OFF)
set(REPROC_OBJECT_LIBRARIES ON)
set(REPROC_INSTALL OFF)
set(REPROC_DEVELOP OFF)
set(REPROC_SANITIZERS OFF)
set(REPROC_TIDY OFF)
set(REPROC_WARNINGS OFF)
set(REPROC_WARNINGS_AS_ERRORS OFF)
add_subdirectory(reproc)
# required to build with extensions disabled
target_compile_definitions(reproc PRIVATE "-D_GNU_SOURCE")
message(STATUS "configured reproc.")
