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
endif()

if (SKBUILD_STATE STREQUAL "sdist")
    message(WARNING "sdist mode detected: Dereferencing symlinks to include actual content...\n"
                    "Background: sdist preserves symlinks, which breaks the package when used outside the git repository. We replace them with actual copies.\n"
                    "This modifies the source tree. To restore a clean git state, run:\n"
                    "  git checkout ${CMAKE_CURRENT_SOURCE_DIR}")

    find_package(Python REQUIRED COMPONENTS Interpreter)

    # List of symlinks to replace with actual content
    set(ITEMS_TO_COPY
        "../../core" "core"
        "../../zxing.cmake" "zxing.cmake"
        "../../LICENSE" "LICENSE"
    )

    while(ITEMS_TO_COPY)
        list(POP_FRONT ITEMS_TO_COPY SRC DST)

        set(ABS_SRC "${CMAKE_CURRENT_SOURCE_DIR}/${SRC}")
        set(ABS_DST "${CMAKE_CURRENT_SOURCE_DIR}/${DST}")

        message(STATUS "Copying ${SRC} to ${DST} (dereferencing symlinks)...")

        # Use Python's shutil to copy files/directories and dereference symlinks (including nested ones)
        execute_process(COMMAND "${Python_EXECUTABLE}" -c
            "import shutil, os, sys;
src = sys.argv[1]; dst = sys.argv[2];
# Remove existing destination (symlink or file/dir)
if os.path.lexists(dst):
    if os.path.isdir(dst) and not os.path.islink(dst): shutil.rmtree(dst)
    else: os.remove(dst)
# Copy source to destination, dereferencing symlinks
if os.path.isdir(src):
    shutil.copytree(src, dst, symlinks=False, dirs_exist_ok=True)
else:
    shutil.copy2(src, dst, follow_symlinks=True)"
            "${ABS_SRC}" "${ABS_DST}"
            RESULT_VARIABLE RET
        )

        if(NOT RET EQUAL 0)
            message(FATAL_ERROR "Failed to copy ${SRC} to ${DST}")
        endif()
    endwhile()
endif() # SKBUILD_STATE STREQUAL "sdist"

# Try to import all Python components potentially needed by nanobind
find_package(Python 3.10
    REQUIRED COMPONENTS Interpreter Development.Module
    OPTIONAL_COMPONENTS Development.SABIModule)
zxing_add_package(nanobind nanobind https://github.com/wjakob/nanobind.git v2.11.0)

# build the python module
nanobind_add_module(
    # Name of the extension
    zxingcpp

    # Target the stable ABI for Python 3.12+, which reduces
    # the number of binary wheels that must be built. This
    # does nothing on older Python versions
    STABLE_ABI

    # Source code goes here
    zxing.cpp
)
target_link_libraries(zxingcpp PRIVATE ZXing::ZXing)

nanobind_add_stub(
    zxingcpp_stub
    MODULE zxingcpp
    OUTPUT zxingcpp.pyi
    PYTHON_PATH $<TARGET_FILE_DIR:zxingcpp>
    DEPENDS zxingcpp
)

if (ZXING_READERS AND ZXING_WRITERS)
    add_test(NAME PythonTest COMMAND ${Python_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/test.py -v)
    set_property(TEST PythonTest PROPERTY ENVIRONMENT PYTHONPATH=$<TARGET_FILE_DIR:zxingcpp>)
endif()

# Mimic setup.py behavior for output directory when using skbuild
if (SKBUILD)
    set_target_properties(zxingcpp PROPERTIES
        LIBRARY_OUTPUT_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
        LIBRARY_OUTPUT_DIRECTORY_RELEASE ${CMAKE_CURRENT_SOURCE_DIR}
        LIBRARY_OUTPUT_DIRECTORY_DEBUG ${CMAKE_CURRENT_SOURCE_DIR})
endif()

# Copy ZXing.dll alongside zxingcpp.pyd to solve ImportError during stub generation
if (WIN32 AND BUILD_SHARED_LIBS)
    add_custom_command(TARGET zxingcpp POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
                $<TARGET_FILE:ZXing::ZXing>
                $<TARGET_FILE_DIR:zxingcpp>
        VERBATIM
    )
endif()

# Default package install directory ("zxingcpp"), enables 'import zxingcpp'
if (NOT DEFINED ZXING_PYTHON_INSTALL_LIBDIR)
    set(ZXING_PYTHON_INSTALL_LIBDIR "zxingcpp")
endif()

install(TARGETS zxingcpp
    COMPONENT python
    LIBRARY DESTINATION "${ZXING_PYTHON_INSTALL_LIBDIR}")
install(FILES ${CMAKE_CURRENT_SOURCE_DIR}/init.py
    COMPONENT python
    RENAME __init__.py
    DESTINATION "${ZXING_PYTHON_INSTALL_LIBDIR}")
install(FILES ${CMAKE_CURRENT_BINARY_DIR}/zxingcpp.pyi
    COMPONENT python
    DESTINATION "${ZXING_PYTHON_INSTALL_LIBDIR}")

