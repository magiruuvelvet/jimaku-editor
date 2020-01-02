set(CURRENT_TARGET "UnitTests")

message(STATUS "\nConfiguring target ${CURRENT_TARGET}...")

CreateTarget(${CURRENT_TARGET} EXECUTABLE unit-tests 17)

target_link_libraries(${CURRENT_TARGET}
PRIVATE
    PGSEncoderInterface
    SubtitleParserInterface
    SubtitleRendererInterface
)

target_include_directories(${CURRENT_TARGET}
PRIVATE
    "${PROJECT_SOURCE_DIR}/Libs/bandit"
)

set(UNIT_TEST_TEMPORARY_DIR "${CMAKE_CURRENT_BINARY_DIR}/tmp")

file(MAKE_DIRECTORY "${UNIT_TEST_TEMPORARY_DIR}")

target_compile_definitions(${CURRENT_TARGET} PRIVATE "-DUNIT_TEST_TEMPORARY_DIR=\"${UNIT_TEST_TEMPORARY_DIR}\"")

message(STATUS "Temporary directory for unit tests: ${UNIT_TEST_TEMPORARY_DIR}")

message(STATUS "Configured target ${CURRENT_TARGET}.\n")
