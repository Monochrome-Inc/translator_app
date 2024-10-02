#-------------------------------------------------
#
# Zombie Panic! Source Translator App Qt Project.
# Created: 2018-12-15
# Last Modified: 2024-10-02
#
#-------------------------------------------------

QT      += core gui
QT      += widgets
QT      += core5compat

TARGET = Translator
TEMPLATE = app

RC_FILE = res.rc

# The following define makes your compiler emit warnings if you use
# any feature of Qt which has been marked as deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS

# You can also make your code fail to compile if you use deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

CONFIG += c++11

SOURCES += \
    KeyValues.cpp \
    main.cpp \
    translator.cpp \
    dialogadd.cpp \
    dialogremove.cpp \
    dialogaddplain.cpp \
    dialogoptions.cpp \
    dialogabout.cpp

HEADERS += \
    KeyValues.h \
    translator.h \
    dialogadd.h \
    dialogremove.h \
    dialogaddplain.h \
    dialogoptions.h \
    dialogabout.h

FORMS += \
    translator.ui \
    dialogadd.ui \
    dialogremove.ui \
    dialogaddplain.ui \
    dialogoptions.ui \
    dialogabout.ui

# Custom Theme -- QDarkStyle
RESOURCES += qdarkstyle/style.qrc

# JsonCPP
win32: LIBS += -L$$PWD/json/ -ljsoncpp
#else:win32:CONFIG(debug, debug|release): LIBS += -L$$PWD/json/ -ljsoncppd

INCLUDEPATH += $$PWD/json
DEPENDPATH += $$PWD/json

win32-g++: PRE_TARGETDEPS += $$PWD/json/libjsoncpp.a
else:win32:!win32-g++: PRE_TARGETDEPS += $$PWD/json/jsoncpp.lib
