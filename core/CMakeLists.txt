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

set (ZXING_CORE_INCLUDE
	"${CMAKE_CURRENT_SOURCE_DIR}/src"
CACHE INTERNAL "")

set (ZXING_CORE_DEFINES "" CACHE INTERNAL "")

if (WINRT)
	set (ZXING_CORE_DEFINES "${ZXING_CORE_DEFINES};WINRT" CACHE INTERNAL "")
endif()

set (ZXING_DEFINES_LOCAL "${ZXING_CORE_DEFINES}")

include_directories (
	${ZXING_CORE_INCLUDE}
)

file (GLOB COMMON_FILES
	src/*.h
	src/*.cpp
)

file (GLOB AZTEC_FILES
	src/aztec/*.h
	src/aztec/*.cpp
)

file (GLOB DATAMATRIX_FILES
	src/datamatrix/*.h
	src/datamatrix/*.cpp
)

file (GLOB MAXICODE_FILES
	src/maxicode/*.h
	src/maxicode/*.cpp
)

file (GLOB ONED_FILES
	src/oned/*.h
	src/oned/*.cpp
)

file (GLOB ONED_RSS_FILES
	src/oned/rss/*.h
	src/oned/rss/*.cpp
)

file (GLOB PDF417_FILES
	src/pdf417/*.h
	src/pdf417/*.cpp
)

file (GLOB QRCODE_FILES
	src/qrcode/*.h
	src/qrcode/*.cpp
)

file (GLOB TEXT_CODEC_FILES
	src/textcodec/*.h
	src/textcodec/*.cpp
)

source_group (Sources FILES ${COMMON_FILES})
source_group (Sources\\aztec FILES ${AZTEC_FILES})
source_group (Sources\\datamatrix FILES ${DATAMATRIX_FILES})
source_group (Sources\\maxicode FILES ${MAXICODE_FILES})
source_group (Sources\\oned FILES ${ONED_FILES})
source_group (Sources\\oned\\rss FILES ${ONED_RSS_FILES})
source_group (Sources\\pdf417 FILES ${PDF417_FILES})
source_group (Sources\\qrcode FILES ${QRCODE_FILES})
source_group (Sources\\textcodec FILES ${TEXT_CODEC_FILES})

add_library (ZXingCore
	${COMMON_FILES}
	${AZTEC_FILES}
	${DATAMATRIX_FILES}
	${MAXICODE_FILES}
	${ONED_FILES}
	${ONED_RSS_FILES}
	${PDF417_FILES}
	${QRCODE_FILES}
	${TEXT_CODEC_FILES}
)

set_target_properties (ZXingCore PROPERTIES COMPILE_DEFINITIONS "${ZXING_DEFINES_LOCAL}")
