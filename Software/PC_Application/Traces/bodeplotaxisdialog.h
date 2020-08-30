#ifndef BODEPLOTAXISDIALOG_H
#define BODEPLOTAXISDIALOG_H

#include <QDialog>
#include "tracebodeplot.h"

namespace Ui {
class BodeplotAxisDialog;
}

class BodeplotAxisDialog : public QDialog
{
    Q_OBJECT

public:
    explicit BodeplotAxisDialog(TraceBodePlot *plot);
    ~BodeplotAxisDialog();

private slots:
    void on_buttonBox_accepted();

private:
    Ui::BodeplotAxisDialog *ui;
    TraceBodePlot *plot;
};

#endif // BODEPLOTAXISDIALOG_H
