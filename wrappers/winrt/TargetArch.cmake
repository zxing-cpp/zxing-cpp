# Based on the Qt 5 processor detection code, so should be very accurate
# https://code.qt.io/cgit/qt/qtbase.git/tree/src/corelib/global/qprocessordetection.h
# Currently handles ARM (v5, v6, v7, v8), ARM64, x86 (32/64), ia64, and ppc (32/64)

# Regarding POWER/PowerPC, just as is noted in the Qt source,
# "There are many more known variants/revisions that we do not handle/detect."

set(archdetect_c_code "
#if defined(__arm__) || defined(__TARGET_ARCH_ARM) || defined(_M_ARM) || defined(_M_ARM64) || defined(__aarch64__) || defined(__ARM64__)
  #if defined(__aarch64__) || defined(__ARM64__) || defined(_M_ARM64)
    #error cmake_ARCH ARM64
  #else
    #error cmake_ARCH ARM
  #endif
#elif defined(__x86_64) || defined(__x86_64__) || defined(__amd64) || defined(_M_X64)
    #error cmake_ARCH x64
#elif defined(__ia64) || defined(__ia64__) || defined(_M_IA64)
    #error cmake_ARCH ia64
#elif defined(__i386) || defined(__i386__) || defined(_M_IX86)
    #error cmake_ARCH x86
#else
	#error cmake_ARCH unknown
#endif
")

# Set ppc_support to TRUE before including this file or ppc and ppc64
# will be treated as invalid architectures since they are no longer supported by Apple

message (STATUS ${CMAKE_GENERATOR})

function(get_target_architecture output_var)
	if (MSVC AND (${CMAKE_GENERATOR} MATCHES "X64$"))
		set (ARCH x64)

	elseif (MSVC AND (${CMAKE_GENERATOR} MATCHES "ARM$"))
		set (ARCH ARM)

    elseif (MSVC AND (${CMAKE_GENERATOR} MATCHES "ARM64$"))
		set (ARCH ARM64)
		
    else()
        file(WRITE "${CMAKE_BINARY_DIR}/arch.c" "${archdetect_c_code}")

        enable_language(C)

        # Detect the architecture in a rather creative way...
        # This compiles a small C program which is a series of ifdefs that selects a
        # particular #error preprocessor directive whose message string contains the
        # target architecture. The program will always fail to compile (both because
        # file is not a valid C program, and obviously because of the presence of the
        # #error preprocessor directives... but by exploiting the preprocessor in this
        # way, we can detect the correct target architecture even when cross-compiling,
        # since the program itself never needs to be run (only the compiler/preprocessor)
        try_run(
            run_result_unused
            compile_result_unused
            "${CMAKE_BINARY_DIR}"
            "${CMAKE_BINARY_DIR}/arch.c"
            COMPILE_OUTPUT_VARIABLE ARCH
        )
		
        # Parse the architecture name from the compiler output
        string(REGEX MATCH "cmake_ARCH ([a-zA-Z0-9_]+)" ARCH "${ARCH}")

        # Get rid of the value marker leaving just the architecture name
        string(REPLACE "cmake_ARCH " "" ARCH "${ARCH}")

        # If we are compiling with an unknown architecture this variable should
        # already be set to "unknown" but in the case that it's empty (i.e. due
        # to a typo in the code), then set it to unknown
        if (NOT ARCH AND MSVC)
			set (ARCH x86)
        endif()
    endif()

    set(${output_var} "${ARCH}" PARENT_SCOPE)
endfunction()
