zxing_add_package_stb()

include (GNUInstallDirs)

if (ZXING_WRITERS)
    add_executable (ZXingWriter ZXingWriter.cpp)

    target_link_libraries (ZXingWriter ZXing::ZXing stb::stb)

    add_test(NAME ZXingWriterTest COMMAND ZXingWriter qrcode "I have the best words." test.png)

    install(TARGETS ZXingWriter DESTINATION ${CMAKE_INSTALL_BINDIR})
endif()

if (ZXING_READERS)
    add_executable (ZXingReader ZXingReader.cpp)

    target_link_libraries (ZXingReader ZXing::ZXing stb::stb)

    add_test(NAME ZXingReaderTest COMMAND ZXingReader -fast -format qrcode test.png) # see above

    install(TARGETS ZXingReader DESTINATION ${CMAKE_INSTALL_BINDIR})
endif()

find_package(Qt6 COMPONENTS Gui Multimedia Quick QUIET)
if (NOT (Qt5_FOUND OR Qt6_FOUND))
    message("INFO: Qt (Gui/Multimedia/Quick) not found, skipping Qt examples")
endif()

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

if (ZXING_READERS)
    if (TARGET Qt::Gui)
        add_executable (ZXingQtReader ZXingQtReader.cpp ZXingQtReader.h)
        target_link_libraries(ZXingQtReader ZXing::ZXing Qt::Gui)
    endif()

    if (TARGET Qt::Multimedia AND TARGET Qt::Quick)
        add_executable(ZXingQtCamReader ZXingQtCamReader.cpp ZXingQtCamReader.qrc ZXingQtReader.h)
        target_link_libraries(ZXingQtCamReader ZXing::ZXing Qt::Gui Qt::Multimedia Qt::Quick)
    endif()

    find_package(OpenCV QUIET)
    if (OpenCV_FOUND)
        add_executable (ZXingOpenCV ZXingOpenCV.cpp)
        target_include_directories (ZXingOpenCV PRIVATE ${OpenCV_INCLUDE_DIRS})
        target_link_libraries (ZXingOpenCV ZXing::ZXing ${OpenCV_LIBS})
    else ()
        message("INFO: OpenCV not found, skipping ZXingOpenCV example")
    endif()
endif()

if (ZXING_WRITERS)
    if (TARGET Qt::Gui)
        add_executable (ZXingQtWriter ZXingQtWriter.cpp)
        target_link_libraries(ZXingQtWriter ZXing::ZXing Qt::Gui)
    endif()
endif()
