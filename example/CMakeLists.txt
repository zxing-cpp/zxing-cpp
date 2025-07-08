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
    set_tests_properties(ZXingReaderTest PROPERTIES DEPENDS ZXingWriterTest)

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
        target_compile_definitions(ZXingQtReader
            PRIVATE
                QT_NO_KEYWORDS
                QT_NO_CAST_TO_ASCII
                QT_NO_CAST_FROM_ASCII
                QT_STRICT_ITERATORS
                QT_NO_URL_CAST_FROM_STRING
                QT_NO_CAST_FROM_BYTEARRAY
                QT_USE_QSTRINGBUILDER
                QT_NO_SIGNALS_SLOTS_KEYWORDS
                QT_USE_FAST_OPERATOR_PLUS
                QT_DISABLE_DEPRECATED_BEFORE=0x060400
        )
    endif()

    if (TARGET Qt::Multimedia AND TARGET Qt::Quick)
        add_executable(ZXingQtCamReader ZXingQtCamReader.cpp ZXingQtCamReader.qrc ZXingQtReader.h)
        target_link_libraries(ZXingQtCamReader ZXing::ZXing Qt::Gui Qt::Multimedia Qt::Quick)
        target_compile_definitions(ZXingQtCamReader
            PRIVATE
                QT_NO_KEYWORDS
                QT_NO_CAST_TO_ASCII
                QT_NO_CAST_FROM_ASCII
                QT_STRICT_ITERATORS
                QT_NO_URL_CAST_FROM_STRING
                QT_NO_CAST_FROM_BYTEARRAY
                QT_USE_QSTRINGBUILDER
                QT_NO_SIGNALS_SLOTS_KEYWORDS
                QT_USE_FAST_OPERATOR_PLUS
                QT_DISABLE_DEPRECATED_BEFORE=0x060400
        )
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
