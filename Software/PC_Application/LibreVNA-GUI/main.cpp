#include "appwindow.h"
#include <QtWidgets/QApplication>
#ifdef Q_OS_UNIX
#include <signal.h>
#endif

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

#ifdef Q_OS_UNIX
    signal(SIGINT, tryExitGracefully);
#endif
    auto rc = app->exec();
    return rc;
}
