# CMake generated Testfile for 
# Source directory: /run/media/leemu/D8C48579C4855B20/projects/zxing-cpp/example
# Build directory: /run/media/leemu/D8C48579C4855B20/projects/zxing-cpp/build/example
# 
# This file includes the relevant testing commands required for 
# testing this directory and lists subdirectories to be tested as well.
add_test(ZXingWriterTest "/run/media/leemu/D8C48579C4855B20/projects/zxing-cpp/build/example/ZXingWriter" "qrcode" "I have the best words." "test.png")
set_tests_properties(ZXingWriterTest PROPERTIES  _BACKTRACE_TRIPLES "/run/media/leemu/D8C48579C4855B20/projects/zxing-cpp/example/CMakeLists.txt;10;add_test;/run/media/leemu/D8C48579C4855B20/projects/zxing-cpp/example/CMakeLists.txt;0;")
add_test(ZXingReaderTest "/run/media/leemu/D8C48579C4855B20/projects/zxing-cpp/build/example/ZXingReader" "-fast" "-format" "qrcode" "test.png")
set_tests_properties(ZXingReaderTest PROPERTIES  _BACKTRACE_TRIPLES "/run/media/leemu/D8C48579C4855B20/projects/zxing-cpp/example/CMakeLists.txt;20;add_test;/run/media/leemu/D8C48579C4855B20/projects/zxing-cpp/example/CMakeLists.txt;0;")
