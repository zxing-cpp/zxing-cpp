cmake_minimum_required (VERSION 3.14)


set (CMAKE_CXX_COMPILER /usr/bin/clang++)

project (ZXingFuzz)

set (CMAKE_CXX_STANDARD 20)
set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g -march=native -fsanitize=address,fuzzer")

set (ZXING_WRITERS OLD)
set (ZXING_READERS ON)

add_definitions (-DZXING_BUILD_FOR_TEST)
add_subdirectory (${CMAKE_CURRENT_SOURCE_DIR}/../../core ${CMAKE_BINARY_DIR}/ZXing)

set (TESTS
    DBEDecoder
    DMEncoder
    ReadLinear
    ReadMatrix
    DecodeMatrix
)

foreach (test ${TESTS})
    set (name "fuzz${test}")
    add_executable (${name} "${name}.cpp")
    target_link_libraries (${name} ZXing::ZXing)
endforeach()

