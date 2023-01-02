cmake_minimum_required(VERSION 3.14)
project(ZXingPython)

set (pybind11_git_repo https://github.com/pybind/pybind11.git)
set (pybind11_git_rev v2.10.2)

# check if we are called from the top-level ZXing project
get_directory_property(hasParent PARENT_DIRECTORY)
if (NOT hasParent)
    # Force to build by C++17. The zxing-cpp require C++17 to build
    set(CMAKE_CXX_STANDARD 17)
    set(CMAKE_CXX_STANDARD_REQUIRED ON)

    option (BUILD_SHARED_LIBS "Link python module to shared lib" OFF)
    option (BUILD_WRITERS "Build with writer support (encoders)" ON)
    option (BUILD_READERS "Build with reader support (decoders)" ON)

    # build the main library
    if (EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/../../core)
        # In development mode, when the whole zxing-cpp directory is checked out, build against head code.
        add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/../../core ZXing EXCLUDE_FROM_ALL)

        include(${CMAKE_CURRENT_SOURCE_DIR}/../../zxing.cmake)
        zxing_add_package(pybind11 pybind11 ${pybind11_git_repo} ${pybind11_git_rev})
    else()
        # we don't have access to the top-level cmake helpers -> simply fetch it unconditional
        include(FetchContent)
        FetchContent_Declare (pybind11
            GIT_REPOSITORY ${pybind11_git_repo}
            GIT_TAG ${pybind11_git_rev})
        FetchContent_MakeAvailable (pybind11)

        # Building from python source distribution (which does not include the whole repository but only python part)
        # so we need to get c++ source git to build the python extension. The python distribution version (given in
        # VERSION_INFO), matches the c++ version the distribution must build against ; hence the checkout of
        # related tag.
        find_package (Git REQUIRED)
        execute_process(
            COMMAND ${GIT_EXECUTABLE} ls-remote https://github.com/zxing-cpp/zxing-cpp.git tags/*
            RESULT_VARIABLE RESULT
            OUTPUT_VARIABLE OUTPUT)
        string (FIND "${OUTPUT}" "refs/tags/v${VERSION_INFO}\n" FOUND_IDX)
        if (FOUND_IDX LESS 0)
            # Version not found in tags (suppose we are preparing future version), use master branch
            FetchContent_Declare(zxing-cpp
                GIT_REPOSITORY https://github.com/zxing-cpp/zxing-cpp.git)
        else()
            FetchContent_Declare(zxing-cpp
                GIT_REPOSITORY https://github.com/zxing-cpp/zxing-cpp.git
                GIT_TAG v${VERSION_INFO})
        endif()
        if(NOT zxing-cpp_POPULATED)
            FetchContent_Populate(zxing-cpp)
            add_subdirectory(${zxing-cpp_SOURCE_DIR}/core ZXing EXCLUDE_FROM_ALL)
        endif()
    endif()
else()
    zxing_add_package(pybind11 pybind11 ${pybind11_git_repo} ${pybind11_git_rev})
endif()

# build the python module
pybind11_add_module(zxingcpp zxing.cpp)
target_link_libraries(zxingcpp PRIVATE ZXing::ZXing)

if (BUILD_READERS AND BUILD_WRITERS)
    add_test(NAME PythonTest COMMAND ${PYTHON_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/test.py -v)
    set_property(TEST PythonTest PROPERTY ENVIRONMENT PYTHONPATH=$<TARGET_FILE_DIR:zxingcpp>)
endif()

install(TARGETS zxingcpp
    COMPONENT python
    RUNTIME DESTINATION "${CMAKE_INSTALL_BINDIR}"
    LIBRARY DESTINATION "${CMAKE_INSTALL_LIBDIR}"
    ARCHIVE DESTINATION "${CMAKE_INSTALL_LIBDIR}")
