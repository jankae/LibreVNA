#include "appwindow.h"
#include "Calibration/calkit.h"
#include "touchstone.h"

#include <iostream>
#include <unistd.h>
#include <QtWidgets/QApplication>
#include <signal.h>
#include <complex>

static QApplication *app;
static AppWindow *window;

void sig_handler(int s) {
    Q_UNUSED(s)
    window->close();
    app->quit();
}

int main(int argc, char *argv[]) {
    app = new QApplication(argc, argv);
    QCoreApplication::setOrganizationName("LibreVNA");
    QCoreApplication::setApplicationName("LibreVNA-GUI");
    window = new AppWindow;
    QCoreApplication::setApplicationVersion(window->getAppVersion() + "-" +
                                            window->getAppGitHash().left(9));
    signal(SIGINT, sig_handler);
    auto rc = app->exec();
    return rc;
}
