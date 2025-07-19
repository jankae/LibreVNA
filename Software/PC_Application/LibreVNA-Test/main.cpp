#include "utiltests.h"
#include "portextensiontests.h"
#include "parametertests.h"
#include "ffttests.h"
#include "impedancerenormalizationtests.h"
#include "calibrationtests.h"

#include <QtTest>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    int status = 0;
    status |= QTest::qExec(new UtilTests, argc, argv);
    status |= QTest::qExec(new PortExtensionTests, argc, argv);
    status |= QTest::qExec(new ParameterTests, argc, argv);
    status |= QTest::qExec(new fftTests, argc, argv);
    status |= QTest::qExec(new ImpedanceRenormalizationTests, argc, argv);
    status |= QTest::qExec(new CalibrationTests, argc, argv);

    return status;
}
