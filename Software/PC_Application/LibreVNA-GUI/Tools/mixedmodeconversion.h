#ifndef MIXEDMODECONVERSION_H
#define MIXEDMODECONVERSION_H

#include "Traces/tracemodel.h"

#include <QDialog>

namespace Ui {
class MixedModeConversion;
}

class MixedModeConversion : public QDialog
{
    Q_OBJECT

public:
    explicit MixedModeConversion(TraceModel &m, QWidget *parent = nullptr);
    ~MixedModeConversion();

signals:
    void tracesCreated(std::vector<Trace*> traces);

private slots:
    void selectionChanged();
private:
    Ui::MixedModeConversion *ui;
    std::vector<Trace*> traces;
};

#endif // MIXEDMODECONVERSION_H
