set(CURRENT_TARGET "PGSEncoder")

message(STATUS "\nConfiguring target ${CURRENT_TARGET}...")

CreateTarget(${CURRENT_TARGET} EXECUTABLE PGSEncoder 0)

# find libpng
pkg_check_modules(LIBPNG REQUIRED libpng)
message(STATUS "libpng library: ${LIBPNG_LIBRARIES}")
message(STATUS "libpng include directory: ${LIBPNG_INCLUDE_DIRS}")

set(PGSENCODER_INCLUDE_DIR "${CURRENT_TARGET_DIR}")
set(PGSENCODER_INCLUDE_DIR "${CURRENT_TARGET_DIR}" PARENT_SCOPE)
message(STATUS "${CURRENT_TARGET} include directory: ${PGSENCODER_INCLUDE_DIR}")

target_include_directories(${CURRENT_TARGET} PRIVATE ${LIBPNG_INCLUDE_DIRS})

target_link_libraries(${CURRENT_TARGET}
PUBLIC
    m
PRIVATE
    ${LIBPNG_LIBRARIES}
)

set_target_properties(${CURRENT_TARGET} PROPERTIES PREFIX "")
set_target_properties(${CURRENT_TARGET} PROPERTIES OUTPUT_NAME "pgssup")

#add_library(PGSEncoderInterface INTERFACE)
#target_include_directories(PGSEncoderInterface INTERFACE "${PGSENCODER_INCLUDE_DIR}")
#target_link_libraries(PGSEncoderInterface INTERFACE PGSEncoder)

message(STATUS "Configured target ${CURRENT_TARGET}.\n")
