HEADERS += \
    ../VNA_embedded/Application/Communication/Protocol.hpp \
    Calibration/calibration.h \
    Calibration/calibrationtracedialog.h \
    Calibration/calkit.h \
    Calibration/calkitdialog.h \
    Calibration/json.hpp \
    Calibration/measurementmodel.h \
    CustomWidgets/colorpickerbutton.h \
    CustomWidgets/siunitedit.h \
    CustomWidgets/tilewidget.h \
    CustomWidgets/toggleswitch.h \
    CustomWidgets/touchstoneimport.h \
    Device/device.h \
    Device/devicelog.h \
    Device/firmwareupdatedialog.h \
    Device/manualcontroldialog.h \
    Generator/generator.h \
    Generator/signalgenwidget.h \
    SpectrumAnalyzer/spectrumanalyzer.h \
    Tools/eseries.h \
    Tools/impedancematchdialog.h \
    Traces/fftcomplex.h \
    Traces/markerwidget.h \
    Traces/trace.h \
    Traces/traceeditdialog.h \
    Traces/traceexportdialog.h \
    Traces/traceimportdialog.h \
    Traces/tracemarker.h \
    Traces/tracemarkermodel.h \
    Traces/tracemodel.h \
    Traces/traceplot.h \
    Traces/tracesmithchart.h \
    Traces/tracewidget.h \
    Traces/tracexyplot.h \
    Traces/xyplotaxisdialog.h \
    Util/qpointervariant.h \
    VNA/portextension.h \
    VNA/vna.h \
    appwindow.h \
    averaging.h \
    mode.h \
    preferences.h \
    qwtplotpiecewisecurve.h \
    touchstone.h \
    unit.h

SOURCES += \
    ../VNA_embedded/Application/Communication/Protocol.cpp \
    Calibration/calibration.cpp \
    Calibration/calibrationtracedialog.cpp \
    Calibration/calkit.cpp \
    Calibration/calkitdialog.cpp \
    Calibration/measurementmodel.cpp \
    CustomWidgets/colorpickerbutton.cpp \
    CustomWidgets/qwtplotpiecewisecurve.cpp \
    CustomWidgets/siunitedit.cpp \
    CustomWidgets/tilewidget.cpp \
    CustomWidgets/toggleswitch.cpp \
    CustomWidgets/touchstoneimport.cpp \
    Device/device.cpp \
    Device/devicelog.cpp \
    Device/firmwareupdatedialog.cpp \
    Device/manualcontroldialog.cpp \
    Generator/generator.cpp \
    Generator/signalgenwidget.cpp \
    SpectrumAnalyzer/spectrumanalyzer.cpp \
    Tools/eseries.cpp \
    Tools/impedancematchdialog.cpp \
    Traces/fftcomplex.cpp \
    Traces/markerwidget.cpp \
    Traces/trace.cpp \
    Traces/traceeditdialog.cpp \
    Traces/traceexportdialog.cpp \
    Traces/traceimportdialog.cpp \
    Traces/tracemarker.cpp \
    Traces/tracemarkermodel.cpp \
    Traces/tracemodel.cpp \
    Traces/traceplot.cpp \
    Traces/tracesmithchart.cpp \
    Traces/tracewidget.cpp \
    Traces/tracexyplot.cpp \
    Traces/xyplotaxisdialog.cpp \
    VNA/portextension.cpp \
    VNA/vna.cpp \
    appwindow.cpp \
    averaging.cpp \
    main.cpp \
    mode.cpp \
    preferences.cpp \
    touchstone.cpp \
    unit.cpp

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
    Generator/signalgenwidget.ui \
    Tools/impedancematchdialog.ui \
    Traces/markerwidget.ui \
    Traces/smithchartdialog.ui \
    Traces/traceeditdialog.ui \
    Traces/traceexportdialog.ui \
    Traces/traceimportdialog.ui \
    Traces/tracewidget.ui \
    Traces/xyplotaxisdialog.ui \
    VNA/portextensioneditdialog.ui \
    main.ui \
    preferencesdialog.ui

DISTFILES +=

RESOURCES += \
    icons.qrc

CONFIG += c++14
REVISION = $$system(git rev-parse HEAD)
DEFINES += GITHASH=\\"\"$$REVISION\\"\"
