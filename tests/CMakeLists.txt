set(CURRENT_TARGET "unit-tests")

CreateTarget(${CURRENT_TARGET} EXECUTABLE unit-tests C++ 17)

target_link_libraries(${CURRENT_TARGET}
PRIVATE
    SubtitleParserInterface
    SubtitleRendererInterface
)

set(UNIT_TEST_CURRENT_DIR "${CMAKE_CURRENT_SOURCE_DIR}")
set(UNIT_TEST_TEMPORARY_DIR "${CMAKE_CURRENT_BINARY_DIR}/tmp")

file(MAKE_DIRECTORY "${UNIT_TEST_TEMPORARY_DIR}")

target_compile_definitions(${CURRENT_TARGET} PRIVATE "-DUNIT_TEST_CURRENT_DIR=\"${UNIT_TEST_CURRENT_DIR}\"")
target_compile_definitions(${CURRENT_TARGET} PRIVATE "-DUNIT_TEST_TEMPORARY_DIR=\"${UNIT_TEST_TEMPORARY_DIR}\"")

message(STATUS "Directory of unit tests source code: ${UNIT_TEST_CURRENT_DIR}")
message(STATUS "Temporary directory for unit tests: ${UNIT_TEST_TEMPORARY_DIR}")
