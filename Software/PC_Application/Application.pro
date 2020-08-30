HEADERS += \
    ../VNA_embedded/Application/Communication/Protocol.hpp \
    Calibration/calibration.h \
    Calibration/calibrationtracedialog.h \
    Calibration/calkit.h \
    Calibration/calkitdialog.h \
    Calibration/measurementmodel.h \
    CustomWidgets/siunitedit.h \
    CustomWidgets/tilewidget.h \
    CustomWidgets/toggleswitch.h \
    CustomWidgets/touchstoneimport.h \
    Device/device.h \
    Device/devicelog.h \
    Device/firmwareupdatedialog.h \
    Device/manualcontroldialog.h \
    Menu/menu.h \
    Menu/menuaction.h \
    Menu/menubool.h \
    Menu/menuitem.h \
    Menu/menuvalue.h \
    Tools/eseries.h \
    Tools/impedancematchdialog.h \
    Traces/bodeplotaxisdialog.h \
    Traces/markerwidget.h \
    Traces/trace.h \
    Traces/tracebodeplot.h \
    Traces/traceeditdialog.h \
    Traces/traceexportdialog.h \
    Traces/traceimportdialog.h \
    Traces/tracemarker.h \
    Traces/tracemarkermodel.h \
    Traces/tracemodel.h \
    Traces/traceplot.h \
    Traces/tracesmithchart.h \
    Traces/tracewidget.h \
    averaging.h \
    qwtplotpiecewisecurve.h \
    touchstone.h \
    unit.h \
    valueinput.h \
    vna.h

SOURCES += \
    ../VNA_embedded/Application/Communication/Protocol.cpp \
    Calibration/calibration.cpp \
    Calibration/calibrationtracedialog.cpp \
    Calibration/calkit.cpp \
    Calibration/calkitdialog.cpp \
    Calibration/measurementmodel.cpp \
    CustomWidgets/siunitedit.cpp \
    CustomWidgets/tilewidget.cpp \
    CustomWidgets/toggleswitch.cpp \
    CustomWidgets/touchstoneimport.cpp \
    Device/device.cpp \
    Device/devicelog.cpp \
    Device/firmwareupdatedialog.cpp \
    Device/manualcontroldialog.cpp \
    Menu/menu.cpp \
    Menu/menuaction.cpp \
    Menu/menubool.cpp \
    Menu/menuitem.cpp \
    Menu/menuvalue.cpp \
    Tools/eseries.cpp \
    Tools/impedancematchdialog.cpp \
    Traces/bodeplotaxisdialog.cpp \
    Traces/markerwidget.cpp \
    Traces/trace.cpp \
    Traces/tracebodeplot.cpp \
    Traces/traceeditdialog.cpp \
    Traces/traceexportdialog.cpp \
    Traces/traceimportdialog.cpp \
    Traces/tracemarker.cpp \
    Traces/tracemarkermodel.cpp \
    Traces/tracemodel.cpp \
    Traces/traceplot.cpp \
    Traces/tracesmithchart.cpp \
    Traces/tracewidget.cpp \
    averaging.cpp \
    main.cpp \
    qwtplotpiecewisecurve.cpp \
    touchstone.cpp \
    unit.cpp \
    valueinput.cpp \
    vna.cpp

LIBS += -lusb-1.0
unix:INCLUDEPATH += /usr/include/qwt
unix:LIBS += -L/usr/lib/ -lqwt-qt5
win32:INCLUDEPATH += C:\Qwt-6.1.4\include
win32:LIBS += -LC:\Qwt-6.1.4\lib -lqwt

QT += widgets

FORMS += \
    Calibration/calibrationtracedialog.ui \
    Calibration/calkitdialog.ui \
    CustomWidgets/tilewidget.ui \
    CustomWidgets/touchstoneimport.ui \
    Device/devicelog.ui \
    Device/firmwareupdatedialog.ui \
    Device/manualcontroldialog.ui \
    Tools/impedancematchdialog.ui \
    Traces/bodeplotaxisdialog.ui \
    Traces/markerwidget.ui \
    Traces/traceeditdialog.ui \
    Traces/traceexportdialog.ui \
    Traces/traceimportdialog.ui \
    Traces/tracewidget.ui \
    main.ui

DISTFILES +=

RESOURCES += \
    icons.qrc

CONFIG += c++14
