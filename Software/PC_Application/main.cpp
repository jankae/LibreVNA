#include <iostream>

#include <unistd.h>
#include <QtWidgets/QApplication>
#include "appwindow.h"

#include "Calibration/calkit.h"
#include "touchstone.h"

#include <complex>
int main(int argc, char *argv[]) {
    QApplication a(argc, argv);
    AppWindow vna;
    vna.resize(1280, 800);
    vna.show();
    a.exec();
}
