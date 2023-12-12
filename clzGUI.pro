QT -= gui
QT += widgets core network websockets sql

TEMPLATE = lib
DEFINES += CLZGUI_LIBRARY
DEFINES += HARDWARE_SPEED
CONFIG += c++17

# The following define makes your compiler emit warnings if you use
# any Qt feature that has been marked deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS
#DEFINES += USE_AOSK
# You can also make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

include($$_PRO_FILE_PWD_/projects/imgui.pri)

INCLUDEPATH += $$_PRO_FILE_PWD_/3rdparty
INCLUDEPATH += $$_PRO_FILE_PWD_/3rdparty/imgui
INCLUDEPATH += $$_PRO_FILE_PWD_/3rdparty/imgui/backends
INCLUDEPATH += $$_PRO_FILE_PWD_/3rdparty/implot
INCLUDEPATH += $$_PRO_FILE_PWD_/3rdparty/ffmpeg-master-latest-win64-gpl-shared/include
INCLUDEPATH += $$_PRO_FILE_PWD_/3rdparty/armadillo-9.850.1/include

LIBS += -L$$_PRO_FILE_PWD_/3rdparty/ffmpeg-master-latest-win64-gpl-shared/lib\
    -lavcodec -lavdevice -lavfilter -lavformat -lavutil -lswresample -lswscale

HEADERS += \
    ClzCore/ClzObject.h \
    ClzCore/ClzObjectManager.h \
    ClzWidgets/ClzCardWidget.h \
    ClzWidgets/ClzClickableFrame.h \
    ClzWidgets/ClzHLayout.h \
    ClzWidgets/ClzImageButton.h \
    ClzWidgets/ClzLayout.h \
    ClzWidgets/ClzLinkStatusWidget.h \
    ClzWidgets/ClzStatusWidget.h \
    ClzWidgets/ClzVLayout.h \
    ClzWidgets/ClzWidget.h \
    Database/ClzDataBaseManager.h \
    Database/ClzDataBaseOnlineCache.h \
    Database/clz_database_common.h \
    Element/ElementContainer.h \
    Element/GraphicsDrawElement.h \
    Element/GraphicsElement.h \
    Element/ImageDrawElement.h \
    Element/Layer/GraphicsLayer.h \
    Element/LinestringDrawElement.h \
    Element/element_include.h \
    GeoFunction/GeoFunctionUtility.h \
    GeoFunction/GeoMapGlobal.h \
    GeoFunction/LayerBackend/GeoBackend.h \
    GeoFunction/LayerBackend/GeoBackendGaode.h \
    GeoFunction/LayerBackend/GeoBackendGoogle.h \
    GeoFunction/LayerBackend/GeoBackendOSM.h \
    Map/GeoMap.h \
    Map/MapItem/MapCopyrightItem.h \
    Map/MapItem/MapItem.h \
    Map/MapItem/MapLabelItem.h \
    Map/MapItem/MapScaleItem.h \
    Map/Projection/Projection.h \
    Map/Projection/ProjectionESG3857.h \
    Map/Projection/ProjectionGCJ02.h \
    Map/TileManager.h \
    Map/map_include.h \
    MapThread/MapRequestThread.h \
    Operator/OperatorBase.h \
    Operator/OperatorContainer.h \
    Operator/OperatorLineString.h \
    Operator/OperatorMark.h \
    Video/UDPReveiver.h \
    Video/VideoDecode.h \
    Video/VideoStreamPush.h \
    Video/VideoWidget.h \
    clzGUI_global.h \
    common/CherileeCommon.h \
    common/CherileeGeomerty.h \
    common/Image.h \
    common/ManagerTemplate.h \
    common/clz_geomap_global.h \
    common/vulkanwindow.h

# Default rules for deployment.
unix {
    target.path = /usr/lib
}
!isEmpty(target.path): INSTALLS += target

SOURCES += \
    ClzCore/ClzObject.cpp \
    ClzCore/ClzObjectManager.cpp \
    ClzWidgets/ClzCardWidget.cpp \
    ClzWidgets/ClzClickableFrame.cpp \
    ClzWidgets/ClzHLayout.cpp \
    ClzWidgets/ClzImageButton.cpp \
    ClzWidgets/ClzLayout.cpp \
    ClzWidgets/ClzLinkStatusWidget.cpp \
    ClzWidgets/ClzStatusWidget.cpp \
    ClzWidgets/ClzVLayout.cpp \
    ClzWidgets/ClzWidget.cpp \
    Database/ClzDataBaseManager.cpp \
    Database/ClzDataBaseOnlineCache.cpp \
    Element/ElementContainer.cpp \
    Element/GraphicsDrawElement.cpp \
    Element/GraphicsElement.cpp \
    Element/ImageDrawElement.cpp \
    Element/Layer/GraphicsLayer.cpp \
    Element/LinestringDrawElement.cpp \
    GeoFunction/GeoFunctionUtility.cpp \
    GeoFunction/GeoMapGlobal.cpp \
    GeoFunction/LayerBackend/GeoBackend.cpp \
    GeoFunction/LayerBackend/GeoBackendGaode.cpp \
    GeoFunction/LayerBackend/GeoBackendGoogle.cpp \
    GeoFunction/LayerBackend/GeoBackendOSM.cpp \
    Map/GeoMap.cpp \
    Map/MapItem/MapCopyrightItem.cpp \
    Map/MapItem/MapItem.cpp \
    Map/MapItem/MapLabelItem.cpp \
    Map/MapItem/MapScaleItem.cpp \
    Map/Projection/Projection.cpp \
    Map/Projection/ProjectionESG3857.cpp \
    Map/Projection/ProjectionGCJ02.cpp \
    Map/TileManager.cpp \
    MapThread/MapRequestThread.cpp \
    Operator/OperatorBase.cpp \
    Operator/OperatorContainer.cpp \
    Operator/OperatorLineString.cpp \
    Operator/OperatorMark.cpp \
    Video/UDPReveiver.cpp \
    Video/VideoDecode.cpp \
    Video/VideoStreamPush.cpp \
    Video/VideoWidget.cpp \
    common/CherileeCommon.cpp \
    common/CherileeGeomerty.cpp \
    common/Image.cpp \
    common/ManagerTemplate.cpp \
    common/vulkanwindow.cpp

win32 {
    INCLUDEPATH += $$(VULKAN_SDK)/Include
#    DEFINES *= VK_USE_PLATFORM_WIN32_KHR
    CONFIG(debug, debug|release) {
        LIBS *= -L$$(VULKAN_SDK)/Lib -lvulkan-1
    } else {
        LIBS *= -L$$(VULKAN_SDK)/Lib -lvulkan-1
    }
}

CONFIG(debug, debug|release) {
     DESTDIR = $$PWD/bin/lib/debug/
} else {
     DESTDIR = $$PWD/bin/lib/release/
}
