if (${CMAKE_SOURCE_DIR} STREQUAL ${CMAKE_CURRENT_SOURCE_DIR})
	# this is the top level project
	cmake_minimum_required (VERSION 2.8.9)

	project (BarcodeScanner)

	add_definitions (-DUNICODE -D_UNICODE)

endif()

set (BARCODESCANNER_INCLUDE "${CMAKE_CURRENT_SOURCE_DIR}" CACHE INTERNAL "")

add_subdirectory (${CMAKE_CURRENT_SOURCE_DIR}/../../core ${CMAKE_BINARY_DIR}/ZXingCore)
	
set (LOCAL_DEFINES
	"${ZXING_CORE_DEFINES}"
)

include_directories (
	${ZXING_CORE_INCLUDE}
	${BARCODESCANNER_INCLUDE}
)

file (GLOB LOCAL_FILES
	BarcodeScanner.h
	BarcodeScanner.cpp
	iconv.h
	win_iconv.c
)

source_group (Sources FILES ${LOCAL_FILES})

add_library (BarcodeScanner
	${LOCAL_FILES}
)

set_target_properties (BarcodeScanner PROPERTIES COMPILE_DEFINITIONS "${LOCAL_DEFINES}")
target_link_libraries (BarcodeScanner ZXingCore gdiplus.lib)
