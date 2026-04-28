cmake_minimum_required(VERSION 3.16)

find_program(SWIFT_EXECUTABLE NAMES swift)
find_program(SWIFTC_EXECUTABLE NAMES swiftc)

if (SWIFT_EXECUTABLE AND SWIFTC_EXECUTABLE AND ZXING_WRITERS AND ZXING_READERS)
    message(STATUS "Found swift compiler")

    set(SWIFT_BUILD_FLAGS
        -Xcc -I${CMAKE_SOURCE_DIR}/core/src
        -Xcc -I${CMAKE_BINARY_DIR}/core
        -Xlinker -L${CMAKE_BINARY_DIR}/core
        -Xlinker -rpath -Xlinker ${CMAKE_BINARY_DIR}/core
    )
    set(SWIFT_RUN_COMMAND ${CMAKE_COMMAND} -E env ZXING_BUNDLED=1 ${SWIFT_EXECUTABLE} run ${SWIFT_BUILD_FLAGS})

    add_test(NAME SwiftDemoWriter
        COMMAND ${SWIFT_RUN_COMMAND} demo_writer "Hello World" QRCode ${CMAKE_CURRENT_BINARY_DIR}/out.png
        WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
    )

    add_test(NAME SwiftDemoReader
        COMMAND ${SWIFT_RUN_COMMAND} demo_reader ${CMAKE_CURRENT_BINARY_DIR}/out.png
        WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
    )
    set_tests_properties(SwiftDemoReader PROPERTIES DEPENDS SwiftDemoWriter)
else()
    message(STATUS "swift not found - skipping Swift wrapper")
endif()
