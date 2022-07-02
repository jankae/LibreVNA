#ifndef MODEWINDOW_H
#define MODEWINDOW_H

#include "modehandler.h"

namespace Ui {
    class ModeWindow;
}

class ModeWindow: public QWidget
{
    Q_OBJECT
public:
    explicit ModeWindow(ModeHandler* handler, AppWindow* aw, QWidget *parent = nullptr);
    ~ModeWindow();

private:
    ModeHandler* handler;
    Ui::ModeWindow *ui;
    void SetupUi();
    AppWindow* aw;

private slots:
    void ModeCreated(int modeIndex);
    void ModeClosed(int modeIndex);
};

#endif // MODEWINDOW_H
