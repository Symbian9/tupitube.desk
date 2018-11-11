QT += opengl core gui svg xml network
QT += multimedia multimediawidgets printsupport
TEMPLATE = lib
TARGET = tupiworkspace

INSTALLS += target
target.path = /lib/

INCLUDEPATH += /usr/include/qt5/QtMultimedia /usr/include/qt5/QtMultimediaWidgets

CONFIG += dll warn_on

HEADERS += tuppaintarea.h \
           tupconfigurationarea.h \
           tupdocumentview.h \
           tuppaintareastatus.h \
           tupimagedevice.h \
           tuppaintareacommand.h \
           tuplibrarydialog.h \
           tupcolorwidget.h \
           tupbrushstatus.h \
           tuptoolstatus.h \
           tupcanvas.h \
           tupcanvasview.h \
           tupimagedialog.h \
           tupstoryboarddialog.h \
           tuppendialog.h \
           tuponiondialog.h \
           # tupinfowidget.h \
           tupruler.h \
           tupcamerainterface.h \
           tupreflexinterface.h \
           tupbasiccamerainterface.h \
           tupcameradialog.h \
           tupcamerawindow.h \
           tupvideosurface.h \
           tuppapagayoimporter.h \
           tuppapagayodialog.h \
           tupreflexrenderarea.h

SOURCES += tuppaintarea.cpp \
           tupconfigurationarea.cpp \
           tupdocumentview.cpp \
           tuppaintareastatus.cpp \
           tupimagedevice.cpp \
           tuppaintareacommand.cpp \
           tuplibrarydialog.cpp \
           tupcolorwidget.cpp \
           tupbrushstatus.cpp \
           tuptoolstatus.cpp \
           tupcanvas.cpp \
           tupcanvasview.cpp \
           tupimagedialog.cpp \
           tupstoryboarddialog.cpp \
           tuppendialog.cpp \
           tuponiondialog.cpp \
           # tupinfowidget.cpp \
           tupruler.cpp \
           tupcamerainterface.cpp \
           tupreflexinterface.cpp \
           tupbasiccamerainterface.cpp \
           tupcameradialog.cpp \
           tupcamerawindow.cpp \
           tupvideosurface.cpp \
           tuppapagayoimporter.cpp \
           tuppapagayodialog.cpp \
           tupreflexrenderarea.cpp

SHELL_DIR = ../../shell/
INCLUDEPATH += $$SHELL_DIR

PLUGIN_DIR = ../../plugins/export/genericexportplugin
INCLUDEPATH += $$PLUGIN_DIR

SELECTION_DIR = ../../plugins/tools/selection
INCLUDEPATH += $$SELECTION_DIR

POLYLINE_DIR = ../../plugins/tools/polyline
INCLUDEPATH += $$POLYLINE_DIR

FRAMEWORK_DIR = "../../framework"
include($$FRAMEWORK_DIR/framework.pri)

unix {
    STORE_DIR = ../../store/
    INCLUDEPATH += $$STORE_DIR
    LIBS += -L$$STORE_DIR -ltupistore

    LIBBASE_DIR = ../../libbase/
    INCLUDEPATH += $$LIBBASE_DIR
    LIBS += -L$$LIBBASE_DIR -ltupibase

    LIBTUPI_DIR = ../../libtupi/
    INCLUDEPATH += $$LIBTUPI_DIR
    LIBS += -L$$LIBTUPI_DIR -ltupi

    !include(../../../tupiglobal.pri) {
             error("Run ./configure first!")
    }
}

win32 {
    include(../../../win.pri)

    STORE_DIR = ../../store/
    INCLUDEPATH += $$STORE_DIR
    LIBS += -L$$STORE_DIR/release/ -ltupistore

    LIBBASE_DIR = ../../libbase/
    INCLUDEPATH += $$LIBBASE_DIR
    LIBS += -L$$LIBBASE_DIR/release/ -ltupibase

    LIBTUPI_DIR = ../../libtupi/
    INCLUDEPATH += $$LIBTUPI_DIR
    LIBS += -L$$LIBTUPI_DIR/release/ -ltupi 
}
