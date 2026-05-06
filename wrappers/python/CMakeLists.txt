cmake_minimum_required(VERSION 3.18)
project(ZXingPython)

# check if we are called from the top-level ZXing project
get_directory_property(hasParent PARENT_DIRECTORY)
if (NOT hasParent)
    set(CMAKE_CXX_STANDARD 20) # required here for the module itself

    option (BUILD_SHARED_LIBS "Link python module to shared lib" OFF)
    option (ZXING_READERS "Build with reader support (decoders)" ON)
    set    (ZXING_WRITERS "NEW" CACHE STRING "Build with old and/or new writer (encoder) backend (OFF/ON/OLD/NEW)")
    set    (ZXING_DEPENDENCIES "AUTO" CACHE STRING "Fetch from github or use locally installed (AUTO/GITHUB/LOCAL)")
    option (ZXING_EXPERIMENTAL_API "Build with experimental API" ON)

    set(CORE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/core)
    if(IS_SYMLINK ${CORE_PATH})
        # This is needed because otherwise GCC resolves the symlink which causes paths to randomly
        # be prefixed by /core or by /wrappers/python/core depending on include order.
        set(CORE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/../../core)
    endif()

    if(EXISTS ${CORE_PATH})
        add_subdirectory(${CORE_PATH} ZXing EXCLUDE_FROM_ALL)
        include(${CMAKE_CURRENT_SOURCE_DIR}/zxing.cmake)
    else()
        message(FATAL_ERROR "Unable to locate zxing source code")
    endif()
    find_package(Python COMPONENTS Interpreter Development.Module REQUIRED)
    zxing_add_package(nanobind nanobind https://github.com/wjakob/nanobind.git v2.12.0)
else()
    find_package(Python COMPONENTS Interpreter Development.Module REQUIRED)
    zxing_add_package(nanobind nanobind https://github.com/wjakob/nanobind.git v2.12.0)
endif()

# build the python module
nanobind_add_module(_zxingcpp_core zxingcpp/zxing.cpp)
target_link_libraries(_zxingcpp_core PRIVATE ZXing::ZXing)
set_target_properties(_zxingcpp_core PROPERTIES
    LIBRARY_OUTPUT_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/zxingcpp")

# Copy __init__.py and py.typed alongside the compiled extension so that
# `import zxingcpp` works directly from the build tree.
add_custom_command(TARGET _zxingcpp_core POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
        "${CMAKE_CURRENT_SOURCE_DIR}/zxingcpp/__init__.py"
        "${CMAKE_CURRENT_BINARY_DIR}/zxingcpp/__init__.py"
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
        "${CMAKE_CURRENT_SOURCE_DIR}/zxingcpp/py.typed"
        "${CMAKE_CURRENT_BINARY_DIR}/zxingcpp/py.typed")

if (ZXING_READERS AND ZXING_WRITERS)
    # Run the test from the build directory so the build-tree zxingcpp/
    # package (with the compiled extension) takes precedence on sys.path
    # over the source-tree zxingcpp/ (which lacks the .so).
    add_custom_command(TARGET _zxingcpp_core POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
            "${CMAKE_CURRENT_SOURCE_DIR}/test.py"
            "${CMAKE_CURRENT_BINARY_DIR}/test.py")
    add_test(NAME PythonTest COMMAND ${Python_EXECUTABLE} ${CMAKE_CURRENT_BINARY_DIR}/test.py -v)
    set_property(TEST PythonTest PROPERTY ENVIRONMENT
        PYTHONPATH=${CMAKE_CURRENT_BINARY_DIR})
endif()

install(TARGETS _zxingcpp_core
    COMPONENT python
    RUNTIME DESTINATION "zxingcpp"
    LIBRARY DESTINATION "zxingcpp"
    ARCHIVE DESTINATION "zxingcpp")

install(FILES
        ${CMAKE_CURRENT_SOURCE_DIR}/zxingcpp/__init__.py
        ${CMAKE_CURRENT_SOURCE_DIR}/zxingcpp/py.typed
    COMPONENT python
    DESTINATION "zxingcpp")

nanobind_add_stub(
    _zxingcpp_core_stub
    MODULE _zxingcpp_core
    OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/zxingcpp/_zxingcpp_core.pyi
    PYTHON_PATH $<TARGET_FILE_DIR:_zxingcpp_core>
    DEPENDS _zxingcpp_core
    COMPONENT python
    VERBOSE)

install(FILES ${CMAKE_CURRENT_BINARY_DIR}/zxingcpp/_zxingcpp_core.pyi
    COMPONENT python
    DESTINATION "zxingcpp")

