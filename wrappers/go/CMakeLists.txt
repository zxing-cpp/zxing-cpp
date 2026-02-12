cmake_minimum_required(VERSION 3.16)

find_program(GO_EXECUTABLE NAMES go)

if (GO_EXECUTABLE AND ZXING_READERS AND ZXING_WRITERS AND ZXING_C_API AND NOT WIN32)
    # Check if Go is available and get version
    execute_process(
        COMMAND ${GO_EXECUTABLE} version
        OUTPUT_VARIABLE GO_VERSION
        OUTPUT_STRIP_TRAILING_WHITESPACE
    )

    if (GO_VERSION)
        message(STATUS "Found Go: ${GO_VERSION}")

        add_test( NAME GoTest
            COMMAND ${GO_EXECUTABLE} test -tags zxing_local -v ./...
            WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
        )
    endif()
elseif (NOT GO_EXECUTABLE)
    message(STATUS "go not found - skipping Go tests")
elseif (WIN32)
    message(STATUS "cgo on Windows can't find cl.exe on GHA - skipping Go tests")
    # using ilammy/msvc-dev-cmd@v1 in cl.yml did not help either
else()
    message(STATUS "ZXING_READERS/ZXING_WRITERS/ZXING_C_API not enabled - skipping Go tests")
endif()
