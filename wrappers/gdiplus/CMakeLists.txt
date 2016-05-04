cmake_minimum_required (VERSION 2.8.9)

project (BarcodeScanner)

add_definitions (-DUNICODE -D_UNICODE)

if (MSVC)
	set (CMAKE_C_FLAGS_RELEASE "${CMAKE_C_FLAGS_RELEASE} /Oi /GS-")
	set (CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} /Oi /GS-")
else()
	set (CMAKE_C_FLAGS_DEBUG "${CMAKE_C_FLAGS_DEBUG} -D_DEBUG")
	set (CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -D_DEBUG")
	if (APPLE)
		set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -stdlib=libc++")
	elseif ("${CMAKE_CXX_COMPILER_ID}" MATCHES "GNU")
		set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")
	endif()
endif()

add_subdirectory (${CMAKE_CURRENT_SOURCE_DIR}/../../core ${CMAKE_BINARY_DIR}/ZXingCore)
	
set (LOCAL_DEFINES "${ZXING_CORE_DEFINES}")

include_directories (
	${ZXING_CORE_INCLUDE}
)

file (GLOB LOCAL_FILES
	*.h
	*.cpp
)

add_executable (BarcodeScanner
	${LOCAL_FILES}
)

set_target_properties (BarcodeScanner PROPERTIES COMPILE_DEFINITIONS "${LOCAL_DEFINES}")
target_link_libraries (BarcodeScanner ZXingCore gdiplus.lib)
