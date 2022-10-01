#ifndef PORTEXTENSIONDIALOG_H
#define PORTEXTENSIONDIALOG_H

#include <QDialog>

namespace Ui {
class PortExtensionDialog;
}

class PortExtensionDialog : public QDialog
{
    Q_OBJECT

public:
    explicit PortExtensionDialog(QWidget *parent = nullptr);
    ~PortExtensionDialog();

private slots:
    void on_buttonBox_accepted();

private:
    Ui::PortExtensionDialog *ui;
};

#endif // PORTEXTENSIONDIALOG_H
