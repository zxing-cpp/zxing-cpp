if (${CMAKE_SOURCE_DIR} STREQUAL ${CMAKE_CURRENT_SOURCE_DIR})
	# this is the top level project
	cmake_minimum_required (VERSION 2.8.9)

	project (ZXingCpp)
	
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
	
endif()

set (ZXING_INCLUDE
	"${CMAKE_CURRENT_SOURCE_DIR}/src"
CACHE INTERNAL "")

set (ZXING_DEFINES "" CACHE INTERNAL "")

if (WINRT)
	set (ZXING_DEFINES "${ZXING_DEFINES};ZX_BUILD_FOR_WINRT;WINRT" CACHE INTERNAL "")
endif()

set (ZXING_DEFINES_LOCAL "${ZXING_DEFINES}")

if (MSVC)
	add_definitions (
		-D_SCL_SECURE_NO_WARNINGS
		-D_CRT_SECURE_NO_WARNINGS
		-D_CRT_NONSTDC_NO_WARNINGS
		-DNOMINMAX
	)
endif()

include_directories (
	${ZXING_INCLUDE}
)

file (GLOB COMMON_FILES
	src/*.h
	src/*.cpp
)

file (GLOB QRCODE_FILES
	src/qrcode/*.h
	src/qrcode/*.cpp
)

file (GLOB DATAMATRIX_FILES
	src/datamatrix/*.h
	src/datamatrix/*.cpp
)

file (GLOB ONED_FILES
	src/oned/*.h
	src/oned/*.cpp
)

file (GLOB ONED_RSS_FILES
	src/oned/rss/*.h
	src/oned/rss/*.cpp
)

source_group (Sources FILES ${COMMON_FILES})
source_group (Sources\\datamatrix FILES ${DATAMATRIX_FILES})
source_group (Sources\\qrcode FILES ${QRCODE_FILES})
source_group (Sources\\oned FILES ${ONED_FILES})
source_group (Sources\\oned\\rss FILES ${ONED_RSS_FILES})

add_library (ZXing
	${COMMON_FILES}
	${DATAMATRIX_FILES}
	${QRCODE_FILES}
	${ONED_FILES}
	${ONED_RSS_FILES}
)

set_target_properties (ZXing PROPERTIES COMPILE_DEFINITIONS "${ZXING_DEFINES_LOCAL}")
