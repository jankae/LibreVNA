#include <iostream>

#include <unistd.h>
#include <QtWidgets/QApplication>
#include "vna.h"
#include "valueinput.h"

#include "Menu/menu.h"
#include "Menu/menuaction.h"
#include "Menu/menuvalue.h"

#include "Calibration/calkit.h"
#include "touchstone.h"

#include <complex>
int main(int argc, char *argv[]) {
    QApplication a(argc, argv);
    VNA vna;
    vna.resize(1280, 800);
    vna.show();
    a.exec();
}
