if (${CMAKE_SOURCE_DIR} STREQUAL ${CMAKE_CURRENT_SOURCE_DIR})
	# this is the top level project
	cmake_minimum_required (VERSION 2.8.9)

	project (ZXingGdiPlus)

	set (ENABLE_ENCODERS OFF CACHE BOOL "Check to include encoders")
	set (ENABLE_DECODERS ON CACHE BOOL "Check to include decoders")

	add_definitions (-DUNICODE -D_UNICODE)

endif()

if (NOT DEFINED ENABLE_ENCODERS)
	set (ENABLE_ENCODERS OFF)
endif()

if (NOT DEFINED ENABLE_DECODERS)
	set (ENABLE_DECODERS ON)
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

if (ENABLE_DECODERS)
	set (LOCAL_FILES
		BarcodeReader.h
		BarcodeReader.cpp
	)
endif()

source_group (Sources FILES ${LOCAL_FILES})

add_library (ZXingGdiPlus
	${LOCAL_FILES}
)

set_target_properties (ZXingGdiPlus PROPERTIES COMPILE_DEFINITIONS "${LOCAL_DEFINES}")
target_link_libraries (ZXingGdiPlus ZXingCore gdiplus.lib)
