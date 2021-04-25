HEADERS += \
    ../VNA_embedded/Application/Communication/Protocol.hpp \
    Calibration/amplitudecaldialog.h \
    Calibration/calibration.h \
    Calibration/calibrationtracedialog.h \
    Calibration/calkit.h \
    Calibration/calkitdialog.h \
    Calibration/manualcalibrationdialog.h \
    Calibration/measurementmodel.h \
    Calibration/receivercaldialog.h \
    Calibration/sourcecaldialog.h \
    CustomWidgets/colorpickerbutton.h \
    CustomWidgets/informationbox.h \
    CustomWidgets/jsonpickerdialog.h \
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
    SpectrumAnalyzer/tracewidgetsa.h \
    Tools/eseries.h \
    Tools/impedancematchdialog.h \
    Tools/parameters.h \
    Traces/Math/dft.h \
    Traces/Math/expression.h \
    Traces/Math/medianfilter.h \
    Traces/Math/parser/mpCompat.h \
    Traces/Math/parser/mpDefines.h \
    Traces/Math/parser/mpError.h \
    Traces/Math/parser/mpFuncCmplx.h \
    Traces/Math/parser/mpFuncCommon.h \
    Traces/Math/parser/mpFuncMatrix.h \
    Traces/Math/parser/mpFuncNonCmplx.h \
    Traces/Math/parser/mpFuncStr.h \
    Traces/Math/parser/mpFwdDecl.h \
    Traces/Math/parser/mpICallback.h \
    Traces/Math/parser/mpIOprt.h \
    Traces/Math/parser/mpIPackage.h \
    Traces/Math/parser/mpIPrecedence.h \
    Traces/Math/parser/mpIToken.h \
    Traces/Math/parser/mpIValReader.h \
    Traces/Math/parser/mpIValue.h \
    Traces/Math/parser/mpIfThenElse.h \
    Traces/Math/parser/mpMatrix.h \
    Traces/Math/parser/mpMatrixError.h \
    Traces/Math/parser/mpOprtBinAssign.h \
    Traces/Math/parser/mpOprtBinCommon.h \
    Traces/Math/parser/mpOprtCmplx.h \
    Traces/Math/parser/mpOprtIndex.h \
    Traces/Math/parser/mpOprtMatrix.h \
    Traces/Math/parser/mpOprtNonCmplx.h \
    Traces/Math/parser/mpOprtPostfixCommon.h \
    Traces/Math/parser/mpPackageCmplx.h \
    Traces/Math/parser/mpPackageCommon.h \
    Traces/Math/parser/mpPackageMatrix.h \
    Traces/Math/parser/mpPackageNonCmplx.h \
    Traces/Math/parser/mpPackageStr.h \
    Traces/Math/parser/mpPackageUnit.h \
    Traces/Math/parser/mpParser.h \
    Traces/Math/parser/mpParserBase.h \
    Traces/Math/parser/mpParserMessageProvider.h \
    Traces/Math/parser/mpRPN.h \
    Traces/Math/parser/mpScriptTokens.h \
    Traces/Math/parser/mpStack.h \
    Traces/Math/parser/mpTest.h \
    Traces/Math/parser/mpTokenReader.h \
    Traces/Math/parser/mpTypes.h \
    Traces/Math/parser/mpValReader.h \
    Traces/Math/parser/mpValue.h \
    Traces/Math/parser/mpValueCache.h \
    Traces/Math/parser/mpVariable.h \
    Traces/Math/parser/suSortPred.h \
    Traces/Math/parser/suStringTokens.h \
    Traces/Math/parser/utGeneric.h \
    Traces/Math/tdr.h \
    Traces/Math/timegate.h \
    Traces/Math/tracemath.h \
    Traces/Math/windowfunction.h \
    Traces/fftcomplex.h \
    Traces/markerwidget.h \
    Traces/sparamtraceselector.h \
    Traces/trace.h \
    Traces/tracecsvexport.h \
    Traces/traceeditdialog.h \
    Traces/traceimportdialog.h \
    Traces/tracemarker.h \
    Traces/tracemarkermodel.h \
    Traces/tracemodel.h \
    Traces/traceplot.h \
    Traces/tracesmithchart.h \
    Traces/tracetouchstoneexport.h \
    Traces/tracewidget.h \
    Traces/tracexyplot.h \
    Traces/xyplotaxisdialog.h \
    Util/qpointervariant.h \
    Util/util.h \
    VNA/Deembedding/deembedding.h \
    VNA/Deembedding/deembeddingdialog.h \
    VNA/Deembedding/deembeddingoption.h \
    VNA/Deembedding/manualdeembeddingdialog.h \
    VNA/Deembedding/matchingnetwork.h \
    VNA/Deembedding/portextension.h \
    VNA/Deembedding/twothru.h \
    VNA/tracewidgetvna.h \
    VNA/vna.h \
    appwindow.h \
    averaging.h \
    csv.h \
    json.hpp \
    mode.h \
    preferences.h \
    savable.h \
    scpi.h \
    tcpserver.h \
    touchstone.h \
    unit.h

SOURCES += \
    ../VNA_embedded/Application/Communication/Protocol.cpp \
    Calibration/amplitudecaldialog.cpp \
    Calibration/calibration.cpp \
    Calibration/calibrationtracedialog.cpp \
    Calibration/calkit.cpp \
    Calibration/calkitdialog.cpp \
    Calibration/manualcalibrationdialog.cpp \
    Calibration/measurementmodel.cpp \
    Calibration/receivercaldialog.cpp \
    Calibration/sourcecaldialog.cpp \
    CustomWidgets/colorpickerbutton.cpp \
    CustomWidgets/informationbox.cpp \
    CustomWidgets/jsonpickerdialog.cpp \
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
    SpectrumAnalyzer/tracewidgetsa.cpp \
    Tools/eseries.cpp \
    Tools/impedancematchdialog.cpp \
    Tools/parameters.cpp \
    Traces/Math/dft.cpp \
    Traces/Math/expression.cpp \
    Traces/Math/medianfilter.cpp \
    Traces/Math/parser/mpError.cpp \
    Traces/Math/parser/mpFuncCmplx.cpp \
    Traces/Math/parser/mpFuncCommon.cpp \
    Traces/Math/parser/mpFuncMatrix.cpp \
    Traces/Math/parser/mpFuncNonCmplx.cpp \
    Traces/Math/parser/mpFuncStr.cpp \
    Traces/Math/parser/mpICallback.cpp \
    Traces/Math/parser/mpIOprt.cpp \
    Traces/Math/parser/mpIPackage.cpp \
    Traces/Math/parser/mpIToken.cpp \
    Traces/Math/parser/mpIValReader.cpp \
    Traces/Math/parser/mpIValue.cpp \
    Traces/Math/parser/mpIfThenElse.cpp \
    Traces/Math/parser/mpOprtBinAssign.cpp \
    Traces/Math/parser/mpOprtBinCommon.cpp \
    Traces/Math/parser/mpOprtCmplx.cpp \
    Traces/Math/parser/mpOprtIndex.cpp \
    Traces/Math/parser/mpOprtMatrix.cpp \
    Traces/Math/parser/mpOprtNonCmplx.cpp \
    Traces/Math/parser/mpOprtPostfixCommon.cpp \
    Traces/Math/parser/mpPackageCmplx.cpp \
    Traces/Math/parser/mpPackageCommon.cpp \
    Traces/Math/parser/mpPackageMatrix.cpp \
    Traces/Math/parser/mpPackageNonCmplx.cpp \
    Traces/Math/parser/mpPackageStr.cpp \
    Traces/Math/parser/mpPackageUnit.cpp \
    Traces/Math/parser/mpParser.cpp \
    Traces/Math/parser/mpParserBase.cpp \
    Traces/Math/parser/mpParserMessageProvider.cpp \
    Traces/Math/parser/mpRPN.cpp \
    Traces/Math/parser/mpScriptTokens.cpp \
    Traces/Math/parser/mpTest.cpp \
    Traces/Math/parser/mpTokenReader.cpp \
    Traces/Math/parser/mpValReader.cpp \
    Traces/Math/parser/mpValue.cpp \
    Traces/Math/parser/mpValueCache.cpp \
    Traces/Math/parser/mpVariable.cpp \
    Traces/Math/tdr.cpp \
    Traces/Math/timegate.cpp \
    Traces/Math/tracemath.cpp \
    Traces/Math/windowfunction.cpp \
    Traces/fftcomplex.cpp \
    Traces/markerwidget.cpp \
    Traces/sparamtraceselector.cpp \
    Traces/trace.cpp \
    Traces/tracecsvexport.cpp \
    Traces/traceeditdialog.cpp \
    Traces/traceimportdialog.cpp \
    Traces/tracemarker.cpp \
    Traces/tracemarkermodel.cpp \
    Traces/tracemodel.cpp \
    Traces/traceplot.cpp \
    Traces/tracesmithchart.cpp \
    Traces/tracetouchstoneexport.cpp \
    Traces/tracewidget.cpp \
    Traces/tracexyplot.cpp \
    Traces/xyplotaxisdialog.cpp \
    VNA/Deembedding/deembedding.cpp \
    VNA/Deembedding/deembeddingdialog.cpp \
    VNA/Deembedding/deembeddingoption.cpp \
    VNA/Deembedding/manualdeembeddingdialog.cpp \
    VNA/Deembedding/matchingnetwork.cpp \
    VNA/Deembedding/portextension.cpp \
    VNA/Deembedding/twothru.cpp \
    VNA/tracewidgetvna.cpp \
    VNA/vna.cpp \
    appwindow.cpp \
    averaging.cpp \
    csv.cpp \
    main.cpp \
    mode.cpp \
    preferences.cpp \
    scpi.cpp \
    tcpserver.cpp \
    touchstone.cpp \
    unit.cpp

LIBS += -lusb-1.0
unix:LIBS += -L/usr/lib/
win32:LIBS += -L"$$_PRO_FILE_PWD_" # Github actions placed libusb here
osx:INCPATH += /usr/local/include
osx:LIBS += $(shell pkg-config --libs libusb-1.0)

#
win32:INCLUDEPATH += C:\libusb-1.0.23\include
win32:LIBS += -LC:\libusb-1.0.23\MinGW64\dll -lusb-1.0
#

QT += widgets network

FORMS += \
    Calibration/addamplitudepointsdialog.ui \
    Calibration/amplitudecaldialog.ui \
    Calibration/automaticamplitudedialog.ui \
    Calibration/calibrationtracedialog.ui \
    Calibration/calkitdialog.ui \
    Calibration/manualcalibrationdialog.ui \
    CustomWidgets/jsonpickerdialog.ui \
    CustomWidgets/tilewidget.ui \
    CustomWidgets/touchstoneimport.ui \
    Device/devicelog.ui \
    Device/firmwareupdatedialog.ui \
    Device/manualcontroldialog.ui \
    Generator/signalgenwidget.ui \
    Tools/impedancematchdialog.ui \
    Traces/Math/dftdialog.ui \
    Traces/Math/dftexplanationwidget.ui \
    Traces/Math/expressiondialog.ui \
    Traces/Math/expressionexplanationwidget.ui \
    Traces/Math/medianexplanationwidget.ui \
    Traces/Math/medianfilterdialog.ui \
    Traces/Math/newtracemathdialog.ui \
    Traces/Math/tdrdialog.ui \
    Traces/Math/tdrexplanationwidget.ui \
    Traces/Math/timedomaingatingexplanationwidget.ui \
    Traces/Math/timegatedialog.ui \
    Traces/Math/timegateexplanationwidget.ui \
    Traces/markerwidget.ui \
    Traces/smithchartdialog.ui \
    Traces/tracecsvexport.ui \
    Traces/traceeditdialog.ui \
    Traces/traceimportdialog.ui \
    Traces/tracetouchstoneexport.ui \
    Traces/tracewidget.ui \
    Traces/xyplotaxisdialog.ui \
    VNA/Deembedding/deembeddingdialog.ui \
    VNA/Deembedding/manualdeembeddingdialog.ui \
    VNA/Deembedding/matchingnetworkdialog.ui \
    VNA/Deembedding/measurementdialog.ui \
    VNA/Deembedding/portextensioneditdialog.ui \
    VNA/Deembedding/twothrudialog.ui \
    VNA/s2pImportOptions.ui \
    main.ui \
    preferencesdialog.ui

DISTFILES +=

RESOURCES += \
    icons.qrc

CONFIG += c++17
REVISION = $$system(git rev-parse HEAD)
DEFINES += GITHASH=\\"\"$$REVISION\\"\"
DEFINES += FW_MAJOR=1 FW_MINOR=0 FW_PATCH=0 FW_SUFFIX=""#\\"\"-alpha.2\\"\"
DEFINES -= _UNICODE UNICODE
