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
    app->quit();
}

int main(int argc, char *argv[]) {
    // Hack: set platform to offscreen if no-gui option specified, this prevents any dialogs from showing up
    char *argv_ext[argc+2];
    for(int i=0;i<argc;i++) {
        argv_ext[i] = argv[i];
    }
    for (int i = 1; i < argc; ++i) {
        if (!qstrcmp(argv[i], "--no-gui")) {
            argv_ext[argc] = const_cast<char*>("-platform");
            argv_ext[argc+1] = const_cast<char*>("offscreen");
            argc+=2;
            break;
        }
    }
    app = new QApplication(argc, argv_ext);
    QCoreApplication::setOrganizationName("LibreVNA");
    QCoreApplication::setApplicationName("LibreVNA-GUI");
    window = new AppWindow;
    QCoreApplication::setApplicationVersion(window->getAppVersion() + "-" +
                                            window->getAppGitHash().left(9));
    signal(SIGINT, sig_handler);
    auto rc = app->exec();
    return rc;
}
