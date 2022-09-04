#include "appwindow.h"
#include <QtWidgets/QApplication>
#include "Device/device.h"
#ifdef Q_OS_UNIX
#include <signal.h>
#endif

#include "Tools/parameters.h"
#include <complex>
using namespace std;

static QApplication *app;
static AppWindow *window;

#ifdef Q_OS_UNIX
static void tryExitGracefully(int s) {
    Q_UNUSED(s)
    window->close();
    app->quit();
}
#endif

int main(int argc, char *argv[]) {

    qSetMessagePattern("%{time process}: [%{type}] %{message}");

    app = new QApplication(argc, argv);
    QCoreApplication::setOrganizationName("LibreVNA");
    QCoreApplication::setApplicationName("LibreVNA-GUI");
    window = new AppWindow;
    QCoreApplication::setApplicationVersion(window->getAppVersion() + "-" +
                                            window->getAppGitHash().left(9));

    Device::RegisterTypes();

    auto S11 = complex<double>(-0.5, 0.25);
    auto S22 = complex<double>(0.5, 0.15);
    auto S33 = complex<double>(0.8, -0.25);

    auto S12 = complex<double>(0.1, 0);
    auto S21 = complex<double>(0.2, 0.3);

    auto S13 = complex<double>(0.3, -0.2);
    auto S31 = complex<double>(0.4, 0.4);

    auto S23 = complex<double>(0.5, 0.2);
    auto S32 = complex<double>(0.6, -0.2);

    auto p12 = Sparam(S11, S12, S21, S22);
    auto p12_only = Sparam(0.0, S12, 1.0, 0.0);
    auto p13 = Sparam(S11, S13, S31, S33);
    auto p23 = Sparam(S22, S23, S32, S33);

    // convert to 75 ohm
    auto p12_75 = Sparam(ABCDparam(p12, 50.0), 75.0);
    auto p12_only_75 = Sparam(ABCDparam(p12_only, 50.0), 75.0);
    auto p13_75 = Sparam(ABCDparam(p12, 50.0), 75.0);
    auto Zp23_75 = Sparam(ABCDparam(p12, 50.0), 75.0);

    auto p1 = Sparam(S11, 0.0, 0.0, 1.0);
    auto p1_75 = Sparam(ABCDparam(p12, 50.0), 75.0);

#ifdef Q_OS_UNIX
    signal(SIGINT, tryExitGracefully);
#endif
    auto rc = app->exec();
    return rc;
}
