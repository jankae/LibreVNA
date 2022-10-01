#ifndef WATERFALLAXISDIALOG_H
#define WATERFALLAXISDIALOG_H

#include "tracewaterfall.h"

#include <QDialog>

namespace Ui {
class WaterfallAxisDialog;
}

class WaterfallAxisDialog : public QDialog
{
    Q_OBJECT

public:
    explicit WaterfallAxisDialog(TraceWaterfall *plot);
    ~WaterfallAxisDialog();

private slots:
    void on_buttonBox_accepted();
    void XAxisTypeChanged(int XAxisIndex);

private:
    std::set<YAxis::Type> supportedYAxis(XAxis::Type type);
    bool isSupported(XAxis::Type type);
    Ui::WaterfallAxisDialog *ui;
    TraceWaterfall *plot;
};

#endif // WATERFALLAXISDIALOG_H
