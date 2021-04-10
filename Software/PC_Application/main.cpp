#include <iostream>

#include <unistd.h>
#include <QtWidgets/QApplication>
#include "appwindow.h"

#include "Calibration/calkit.h"
#include "touchstone.h"

#include <signal.h>

#include <complex>

static QApplication *app;
static AppWindow *window;

void sig_handler(int s) {
    Q_UNUSED(s)
    window->close();
}

int main(int argc, char *argv[]) {
    app = new QApplication(argc, argv);
    window = new AppWindow;
    signal(SIGINT, sig_handler);
    app->exec();
    return 0;
}
