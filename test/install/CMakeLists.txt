cmake_minimum_required(VERSION 3.16)

project(ZXingInstallTest LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17) # make sure the public API is still c++17 compatible

find_package(ZXing CONFIG REQUIRED)

add_executable(InstallTest main.cpp)

target_link_libraries(InstallTest PRIVATE ZXing::ZXing)
