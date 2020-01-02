set(CURRENT_TARGET "PGSEncoder")

message(STATUS "\nConfiguring target ${CURRENT_TARGET}...")

CreateTarget(${CURRENT_TARGET} STATIC PGSEncoder 17)

set(PGSENCODER_INCLUDE_DIR "${CURRENT_TARGET_DIR}")
set(PGSENCODER_INCLUDE_DIR "${CURRENT_TARGET_DIR}" PARENT_SCOPE)
message(STATUS "${CURRENT_TARGET} include directory: ${PGSENCODER_INCLUDE_DIR}")

add_library(PGSEncoderInterface INTERFACE)
target_include_directories(PGSEncoderInterface INTERFACE "${PGSENCODER_INCLUDE_DIR}")

message(STATUS "Configured target ${CURRENT_TARGET}.\n")
