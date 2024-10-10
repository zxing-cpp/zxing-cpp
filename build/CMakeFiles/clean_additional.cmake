# Additional clean files
cmake_minimum_required(VERSION 3.16)

if("${CONFIG}" STREQUAL "" OR "${CONFIG}" STREQUAL "RelWithDebInfo")
  file(REMOVE_RECURSE
  "example/CMakeFiles/ZXingOpenCV_autogen.dir/AutogenUsed.txt"
  "example/CMakeFiles/ZXingOpenCV_autogen.dir/ParseCache.txt"
  "example/CMakeFiles/ZXingQtCamReader_autogen.dir/AutogenUsed.txt"
  "example/CMakeFiles/ZXingQtCamReader_autogen.dir/ParseCache.txt"
  "example/CMakeFiles/ZXingQtReader_autogen.dir/AutogenUsed.txt"
  "example/CMakeFiles/ZXingQtReader_autogen.dir/ParseCache.txt"
  "example/CMakeFiles/ZXingQtWriter_autogen.dir/AutogenUsed.txt"
  "example/CMakeFiles/ZXingQtWriter_autogen.dir/ParseCache.txt"
  "example/ZXingOpenCV_autogen"
  "example/ZXingQtCamReader_autogen"
  "example/ZXingQtReader_autogen"
  "example/ZXingQtWriter_autogen"
  )
endif()
