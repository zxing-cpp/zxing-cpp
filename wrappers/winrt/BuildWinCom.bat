@echo off
rem Path to folder where the result should be copied to
rem This should be the path to the folder containing SDKManifest.xml
set DESTINATION=%~dp0..\..\..\path_to_folder_of_SDKManifest.xml


pushd %DESTINATION%
set DESTINATION=%CD%
popd

set BUILD_LOC=%~dp0..\..\build_uwp_x86
md %BUILD_LOC%
pushd %BUILD_LOC%
cmake -G "Visual Studio 14 2015" -DCMAKE_SYSTEM_NAME=WindowsStore -DCMAKE_SYSTEM_VERSION=10.0 -DEXTENSION_SDK_OUTPUT="%DESTINATION%" ..\wrappers\winrt
cmake --build . --config Release
popd
rd /s /q %BUILD_LOC%

set BUILD_LOC=%~dp0..\..\build_uwp_x64
md %BUILD_LOC%
pushd %BUILD_LOC%
cmake -G "Visual Studio 14 2015 Win64" -DCMAKE_SYSTEM_NAME=WindowsStore -DCMAKE_SYSTEM_VERSION=10.0 -DEXTENSION_SDK_OUTPUT="%DESTINATION%" ..\wrappers\winrt
cmake --build . --config Release
popd
rd /s /q %BUILD_LOC%

set BUILD_LOC=%~dp0..\..\build_uwp_arm
md %BUILD_LOC%
pushd %BUILD_LOC%
cmake -G "Visual Studio 14 2015 ARM" -DCMAKE_SYSTEM_NAME=WindowsStore -DCMAKE_SYSTEM_VERSION=10.0 -DEXTENSION_SDK_OUTPUT="%DESTINATION%" ..\wrappers\winrt
cmake --build . --config Release
popd
rd /s /q %BUILD_LOC%
