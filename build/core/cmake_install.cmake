# Install script for directory: /run/media/leemu/D8C48579C4855B20/projects/zxing-cpp/core

# Set the install prefix
if(NOT DEFINED CMAKE_INSTALL_PREFIX)
  set(CMAKE_INSTALL_PREFIX "/usr/local")
endif()
string(REGEX REPLACE "/$" "" CMAKE_INSTALL_PREFIX "${CMAKE_INSTALL_PREFIX}")

# Set the install configuration name.
if(NOT DEFINED CMAKE_INSTALL_CONFIG_NAME)
  if(BUILD_TYPE)
    string(REGEX REPLACE "^[^A-Za-z0-9_]+" ""
           CMAKE_INSTALL_CONFIG_NAME "${BUILD_TYPE}")
  else()
    set(CMAKE_INSTALL_CONFIG_NAME "RelWithDebInfo")
  endif()
  message(STATUS "Install configuration: \"${CMAKE_INSTALL_CONFIG_NAME}\"")
endif()

# Set the component getting installed.
if(NOT CMAKE_INSTALL_COMPONENT)
  if(COMPONENT)
    message(STATUS "Install component: \"${COMPONENT}\"")
    set(CMAKE_INSTALL_COMPONENT "${COMPONENT}")
  else()
    set(CMAKE_INSTALL_COMPONENT)
  endif()
endif()

# Install shared libraries without execute permission?
if(NOT DEFINED CMAKE_INSTALL_SO_NO_EXE)
  set(CMAKE_INSTALL_SO_NO_EXE "0")
endif()

# Is this installation the result of a crosscompile?
if(NOT DEFINED CMAKE_CROSSCOMPILING)
  set(CMAKE_CROSSCOMPILING "FALSE")
endif()

# Set path to fallback-tool for dependency-resolution.
if(NOT DEFINED CMAKE_OBJDUMP)
  set(CMAKE_OBJDUMP "/usr/bin/objdump")
endif()

if(CMAKE_INSTALL_COMPONENT STREQUAL "Unspecified" OR NOT CMAKE_INSTALL_COMPONENT)
  foreach(file
      "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/libZXing.so.2.2.1"
      "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/libZXing.so.3"
      )
    if(EXISTS "${file}" AND
       NOT IS_SYMLINK "${file}")
      file(RPATH_CHECK
           FILE "${file}"
           RPATH "")
    endif()
  endforeach()
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/lib" TYPE SHARED_LIBRARY FILES
    "/run/media/leemu/D8C48579C4855B20/projects/zxing-cpp/build/core/libZXing.so.2.2.1"
    "/run/media/leemu/D8C48579C4855B20/projects/zxing-cpp/build/core/libZXing.so.3"
    )
  foreach(file
      "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/libZXing.so.2.2.1"
      "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/libZXing.so.3"
      )
    if(EXISTS "${file}" AND
       NOT IS_SYMLINK "${file}")
      if(CMAKE_INSTALL_DO_STRIP)
        execute_process(COMMAND "/usr/bin/strip" "${file}")
      endif()
    endif()
  endforeach()
endif()

if(CMAKE_INSTALL_COMPONENT STREQUAL "Unspecified" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/lib" TYPE SHARED_LIBRARY FILES "/run/media/leemu/D8C48579C4855B20/projects/zxing-cpp/build/core/libZXing.so")
endif()

if(CMAKE_INSTALL_COMPONENT STREQUAL "Unspecified" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/include/ZXing" TYPE FILE FILES
    "/run/media/leemu/D8C48579C4855B20/projects/zxing-cpp/core/src/Barcode.h"
    "/run/media/leemu/D8C48579C4855B20/projects/zxing-cpp/core/src/BarcodeFormat.h"
    "/run/media/leemu/D8C48579C4855B20/projects/zxing-cpp/core/src/BitHacks.h"
    "/run/media/leemu/D8C48579C4855B20/projects/zxing-cpp/core/src/ByteArray.h"
    "/run/media/leemu/D8C48579C4855B20/projects/zxing-cpp/core/src/CharacterSet.h"
    "/run/media/leemu/D8C48579C4855B20/projects/zxing-cpp/core/src/Content.h"
    "/run/media/leemu/D8C48579C4855B20/projects/zxing-cpp/core/src/Error.h"
    "/run/media/leemu/D8C48579C4855B20/projects/zxing-cpp/core/src/Flags.h"
    "/run/media/leemu/D8C48579C4855B20/projects/zxing-cpp/core/src/GTIN.h"
    "/run/media/leemu/D8C48579C4855B20/projects/zxing-cpp/core/src/ImageView.h"
    "/run/media/leemu/D8C48579C4855B20/projects/zxing-cpp/core/src/Point.h"
    "/run/media/leemu/D8C48579C4855B20/projects/zxing-cpp/core/src/Quadrilateral.h"
    "/run/media/leemu/D8C48579C4855B20/projects/zxing-cpp/core/src/ReadBarcode.h"
    "/run/media/leemu/D8C48579C4855B20/projects/zxing-cpp/core/src/ReaderOptions.h"
    "/run/media/leemu/D8C48579C4855B20/projects/zxing-cpp/core/src/StructuredAppend.h"
    "/run/media/leemu/D8C48579C4855B20/projects/zxing-cpp/core/src/TextUtfEncoding.h"
    "/run/media/leemu/D8C48579C4855B20/projects/zxing-cpp/core/src/ZXingCpp.h"
    "/run/media/leemu/D8C48579C4855B20/projects/zxing-cpp/core/src/ZXAlgorithms.h"
    "/run/media/leemu/D8C48579C4855B20/projects/zxing-cpp/core/src/ZXVersion.h"
    "/run/media/leemu/D8C48579C4855B20/projects/zxing-cpp/core/src/DecodeHints.h"
    "/run/media/leemu/D8C48579C4855B20/projects/zxing-cpp/core/src/Result.h"
    "/run/media/leemu/D8C48579C4855B20/projects/zxing-cpp/core/src/BitMatrix.h"
    "/run/media/leemu/D8C48579C4855B20/projects/zxing-cpp/core/src/BitMatrixIO.h"
    "/run/media/leemu/D8C48579C4855B20/projects/zxing-cpp/core/src/Range.h"
    "/run/media/leemu/D8C48579C4855B20/projects/zxing-cpp/core/src/Matrix.h"
    "/run/media/leemu/D8C48579C4855B20/projects/zxing-cpp/core/src/MultiFormatWriter.h"
    )
endif()

if(CMAKE_INSTALL_COMPONENT STREQUAL "Unspecified" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/include/ZXing" TYPE FILE FILES "/run/media/leemu/D8C48579C4855B20/projects/zxing-cpp/build/core/Version.h")
endif()

if(CMAKE_INSTALL_COMPONENT STREQUAL "Unspecified" OR NOT CMAKE_INSTALL_COMPONENT)
  if(EXISTS "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/cmake/ZXing/ZXingTargets.cmake")
    file(DIFFERENT _cmake_export_file_changed FILES
         "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/cmake/ZXing/ZXingTargets.cmake"
         "/run/media/leemu/D8C48579C4855B20/projects/zxing-cpp/build/core/CMakeFiles/Export/f9e04a807b27a41299a115186893fdf1/ZXingTargets.cmake")
    if(_cmake_export_file_changed)
      file(GLOB _cmake_old_config_files "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/cmake/ZXing/ZXingTargets-*.cmake")
      if(_cmake_old_config_files)
        string(REPLACE ";" ", " _cmake_old_config_files_text "${_cmake_old_config_files}")
        message(STATUS "Old export file \"$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/cmake/ZXing/ZXingTargets.cmake\" will be replaced.  Removing files [${_cmake_old_config_files_text}].")
        unset(_cmake_old_config_files_text)
        file(REMOVE ${_cmake_old_config_files})
      endif()
      unset(_cmake_old_config_files)
    endif()
    unset(_cmake_export_file_changed)
  endif()
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/lib/cmake/ZXing" TYPE FILE FILES "/run/media/leemu/D8C48579C4855B20/projects/zxing-cpp/build/core/CMakeFiles/Export/f9e04a807b27a41299a115186893fdf1/ZXingTargets.cmake")
  if(CMAKE_INSTALL_CONFIG_NAME MATCHES "^([Rr][Ee][Ll][Ww][Ii][Tt][Hh][Dd][Ee][Bb][Ii][Nn][Ff][Oo])$")
    file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/lib/cmake/ZXing" TYPE FILE FILES "/run/media/leemu/D8C48579C4855B20/projects/zxing-cpp/build/core/CMakeFiles/Export/f9e04a807b27a41299a115186893fdf1/ZXingTargets-relwithdebinfo.cmake")
  endif()
endif()

if(CMAKE_INSTALL_COMPONENT STREQUAL "Unspecified" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/lib/pkgconfig" TYPE FILE FILES "/run/media/leemu/D8C48579C4855B20/projects/zxing-cpp/build/core/zxing.pc")
endif()

if(CMAKE_INSTALL_COMPONENT STREQUAL "Unspecified" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/lib/cmake/ZXing" TYPE FILE FILES
    "/run/media/leemu/D8C48579C4855B20/projects/zxing-cpp/build/core/ZXingConfig.cmake"
    "/run/media/leemu/D8C48579C4855B20/projects/zxing-cpp/build/core/ZXingConfigVersion.cmake"
    )
endif()

