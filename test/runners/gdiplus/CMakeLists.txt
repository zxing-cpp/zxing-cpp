cmake_minimum_required (VERSION 2.8.9)

project (ReaderTest)

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

add_subdirectory (${CMAKE_CURRENT_SOURCE_DIR}/../../../wrappers/gdiplus ${CMAKE_BINARY_DIR}/BarcodeScanner)
	
include_directories (
	${BARCODESCANNER_INCLUDE}
)

add_executable (ReaderTest
	TestMain.cpp
)

target_link_libraries (ReaderTest BarcodeScanner)
