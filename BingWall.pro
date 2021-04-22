#-------------------------------------------------
#
# Project created by QtCreator 2020-02-17T22:45:35
#
#-------------------------------------------------

QT       += core gui network

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = bing-wall
TEMPLATE = app

# Set program version
VERSION = 0.0.1
DEFINES += VERSIONSTR=\\\"$${VERSION}\\\"

CONFIG(release, debug|release):DEFINES += QT_NO_DEBUG_OUTPUT



# The following define makes your compiler emit warnings if you use
# any feature of Qt which has been marked as deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS

# You can also make your code fail to compile if you use deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

CONFIG += c++11

SOURCES += \
        controlbutton.cpp \
        darkstyle.cpp \
        main.cpp \
        mainwindow.cpp \
        remotepixmaplabel2.cpp \
        request.cpp \
        scrolltext.cpp \
        waitingspinnerwidget.cpp

HEADERS += \
        controlbutton.h \
        darkstyle.h \
        gridlayoututil.h \
        mainwindow.h \
        remotepixmaplabel2.h \
        request.h \
        scrolltext.h \
        waitingspinnerwidget.h

FORMS += \
        action_ui.ui \
        downloaded.ui \
        downloaded_option.ui \
        listitem_ui.ui \
        mainwindow.ui \
        settings.ui

# Default rules for deployment.
isEmpty(PREFIX){
 PREFIX = /usr
}

BINDIR  = $$PREFIX/bin
DATADIR = $$PREFIX/share

target.path = $$BINDIR

icon.files = icons/bingwall.png
icon.path = $$DATADIR/icons/hicolor/512x512/apps/

desktop.files = bingwall.desktop
desktop.path = $$DATADIR/applications/

INSTALLS += target icon desktop


RESOURCES += \
    darkstyle.qrc \
    images.qrc

