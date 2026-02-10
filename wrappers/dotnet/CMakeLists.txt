cmake_minimum_required(VERSION 3.16)

find_program(DOTNET_EXECUTABLE NAMES dotnet)

if (DOTNET_EXECUTABLE AND ZXING_WRITERS AND ZXING_READERS)
    # Check available .NET runtimes
    execute_process(
        COMMAND ${DOTNET_EXECUTABLE} --list-runtimes
        OUTPUT_VARIABLE DOTNET_RUNTIMES
        OUTPUT_STRIP_TRAILING_WHITESPACE
    )

    if (DOTNET_RUNTIMES MATCHES "Microsoft\\.NETCore\\.App (10\\.|1[1-9]\\.|[2-9][0-9])")
        message(STATUS "Found dotnet with net10.0 runtime")

        add_test(NAME DotNetTest COMMAND ${DOTNET_EXECUTABLE} test WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR})

        # Copy native library to test output directories so .NET can find it
        # On macOS, DYLD_LIBRARY_PATH is ignored due to SIP when launching signed binaries
        # The library may have version suffix (e.g., libZXing.3.0.0.dylib), so create symlink to base name
        add_custom_target(DotNetCopyNative ALL
            COMMAND ${CMAKE_COMMAND} -E make_directory ${CMAKE_CURRENT_SOURCE_DIR}/ZXingCpp.Tests/bin/Debug/net10.0
            COMMAND ${CMAKE_COMMAND} -E copy_if_different $<TARGET_FILE:ZXing> ${CMAKE_CURRENT_SOURCE_DIR}/ZXingCpp.Tests/bin/Debug/net10.0/
            COMMAND ${CMAKE_COMMAND} -E create_symlink $<TARGET_FILE_NAME:ZXing> ${CMAKE_CURRENT_SOURCE_DIR}/ZXingCpp.Tests/bin/Debug/net10.0/$<TARGET_LINKER_FILE_NAME:ZXing>
            DEPENDS ZXing
            COMMENT "Copying ZXing library to .NET test output directories"
        )
    else()
        message(STATUS "dotnet found but no compatible runtime available - skipping .NET tests")
    endif()
else()
    message(STATUS "dotnet not found - skipping .NET tests")
endif()
