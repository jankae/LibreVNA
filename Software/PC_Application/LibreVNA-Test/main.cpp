#include "utiltests.h"

#include <QtTest>

int main(int argc, char *argv[])
{
    int status = 0;
    status |= QTest::qExec(new UtilTests, argc, argv);

    return status;
}
