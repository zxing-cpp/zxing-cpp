
find_package(Qt6 COMPONENTS Gui Multimedia Quick Widgets QUIET)
if (NOT (Qt5_FOUND OR Qt6_FOUND))
    message("INFO: Qt (Gui/Multimedia/Quick/Widgets) not found, skipping Qt examples")
endif()

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

set(ZXING_QT_DEFINITIONS
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

if (ZXING_READERS)
    if (TARGET Qt::Gui)
        add_executable (ZXingQtReader ZXingQtReader.cpp ZXingQt.h)
        target_link_libraries(ZXingQtReader ZXing::ZXing Qt::Gui)
        target_compile_definitions(ZXingQtReader PRIVATE ${ZXING_QT_DEFINITIONS})
    endif()

    if (TARGET Qt::Multimedia AND TARGET Qt::Quick)
        add_executable(ZXingQmlReader ZXingQmlReader.cpp ZXingQmlReader.qrc ZXingQt.h)
        target_link_libraries(ZXingQmlReader PRIVATE ZXing::ZXing Qt::Gui Qt::Multimedia Qt::Quick)
        target_compile_definitions(ZXingQmlReader PRIVATE ${ZXING_QT_DEFINITIONS})
        if (APPLE)
            set_target_properties(ZXingQmlReader PROPERTIES
                MACOSX_BUNDLE TRUE
                MACOSX_BUNDLE_INFO_PLIST ${CMAKE_CURRENT_SOURCE_DIR}/ZXingQmlReader-Info.plist
            )
        endif()
        qt_finalize_executable(ZXingQmlReader)
    endif()

    if (TARGET Qt::Multimedia AND TARGET Qt::Widgets)
        add_executable(ZXingQtCamReader ZXingQtCamReader.cpp ZXingQt.h)
        target_link_libraries(ZXingQtCamReader PRIVATE ZXing::ZXing Qt::Gui Qt::Multimedia Qt::Widgets)
        target_compile_definitions(ZXingQtCamReader PRIVATE ${ZXING_QT_DEFINITIONS})
        if (APPLE)
            set_target_properties(ZXingQtCamReader PROPERTIES
                MACOSX_BUNDLE TRUE
                MACOSX_BUNDLE_INFO_PLIST ${CMAKE_CURRENT_SOURCE_DIR}/ZXingQtCamReader-Info.plist
            )
        endif()
        qt_finalize_executable(ZXingQtCamReader)
    endif()

endif()

if (ZXING_WRITERS)
    if (TARGET Qt::Gui)
        add_executable (ZXingQtWriter ZXingQtWriter.cpp)
        target_link_libraries(ZXingQtWriter ZXing::ZXing Qt::Gui)
        target_compile_definitions(ZXingQtWriter PRIVATE ${ZXING_QT_DEFINITIONS})
    endif()
endif()
