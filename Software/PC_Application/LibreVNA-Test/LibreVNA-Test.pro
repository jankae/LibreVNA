QT += testlib
#QT -= gui

CONFIG += qt console warn_on depend_includepath testcase
CONFIG -= app_bundle

TEMPLATE = app

SOURCES +=  \
    ../LibreVNA-GUI/Util/util.cpp \
    main.cpp \
    utiltests.cpp

HEADERS += \
    ../LibreVNA-GUI/Util/util.h \
    utiltests.h

INCLUDEPATH += \
    ../LibreVNA-GUI/Util
