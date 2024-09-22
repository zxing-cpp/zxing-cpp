
macro(zxing_add_package_stb)
    unset (STB_FOUND CACHE)

    if (ZXING_DEPENDENCIES STREQUAL "AUTO")
        find_package(PkgConfig)
        pkg_check_modules (STB IMPORTED_TARGET stb)
    elseif (ZXING_DEPENDENCIES STREQUAL "LOCAL")
        find_package(PkgConfig REQUIRED)
        pkg_check_modules (STB REQUIRED IMPORTED_TARGET stb)
    endif()

    if (NOT STB_FOUND)
        include(FetchContent)
        FetchContent_Declare (stb
            GIT_REPOSITORY https://github.com/nothings/stb.git)
        FetchContent_MakeAvailable (stb)
        add_library(stb::stb INTERFACE IMPORTED)
        target_include_directories(stb::stb INTERFACE ${stb_SOURCE_DIR})
    else()
        add_library(stb::stb ALIAS PkgConfig::STB)
    endif()
endmacro()

macro(zxing_add_package name depname git_repo git_rev)
    unset(${name}_FOUND CACHE) # see https://github.com/zxing-cpp/zxing-cpp/commit/8db14eeead45e0f1961532f55061d5e4dd0f78be#commitcomment-66464026

    if (ZXING_DEPENDENCIES STREQUAL "AUTO")
        find_package (${name} CONFIG)
    elseif (ZXING_DEPENDENCIES STREQUAL "LOCAL")
        find_package (${name} REQUIRED CONFIG)
    endif()

    if (NOT ${name}_FOUND)
        include(FetchContent)
        FetchContent_Declare (${depname}
            GIT_REPOSITORY ${git_repo}
            GIT_TAG ${git_rev})
        if (${depname} STREQUAL "googletest")
            # Prevent overriding the parent project's compiler/linker settings on Windows
            set (gtest_force_shared_crt ON CACHE BOOL "" FORCE)
        endif()
        #FetchContent_MakeAvailable (${depname})
        # TODO: reqire cmake 3.28 and pass EXCLUDE_FROM_ALL to FetchContent_Declare instead of calling FetchContent_Populate below.
        #       This would fix a warning but cmake 3.28 is not in Debian bookworm but at least in Ubuntu 24.04 (LTS)
        FetchContent_GetProperties(${depname})
        if(NOT ${depname}_POPULATED)
            FetchContent_Populate(${depname})
            add_subdirectory(${${depname}_SOURCE_DIR} ${${depname}_BINARY_DIR} EXCLUDE_FROM_ALL) # prevent installing of dependencies
        endif()
        set (${name}_POPULATED TRUE) # this is supposed to be done in MakeAvailable but it seems not to?!?
    endif()
endmacro()
