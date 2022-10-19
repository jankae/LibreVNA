#ifndef EYEDIAGRAMDIALOG_H
#define EYEDIAGRAMDIALOG_H

#include "Traces/tracemodel.h"
#include "Traces/Math/tdr.h"

#include <vector>

#include <QDialog>

namespace Ui {
class EyeDiagramDialog;
}

class EyeDiagramDialog;

class EyeDiagramPlot : public QWidget
{
    Q_OBJECT
public:
    EyeDiagramPlot(QDialog *dialog);

    void setDialog(EyeDiagramDialog *dialog);

    unsigned int eyeWidth();
    unsigned int eyeHeight();
private:
    unsigned int leftSpace();
    unsigned int rightSpace() {return 10;}
    unsigned int topSpace() {return 10;}
    unsigned int bottomSpace();
    void paintEvent(QPaintEvent *event) override;

    EyeDiagramDialog *dialog;
};

class EyeDiagramDialog : public QDialog
{
    Q_OBJECT

public:
    explicit EyeDiagramDialog(TraceModel &model);
    ~EyeDiagramDialog();

    unsigned int getCalculatedPixelsX();
    unsigned int getCalculatedPixelsY();
    double getIntensity(unsigned int x, unsigned int y);

    double displayedTime();
    double minGraphVoltage();
    double maxGraphVoltage();

public slots:
    bool triggerUpdate();
    bool update(unsigned int width, unsigned int height);

signals:
    void updateDone();

private:
signals:
    void calculationStatus(QString s);

private:
    static constexpr double yOverrange = 0.2;
    void updateThread(unsigned int width, unsigned int height);

    Ui::EyeDiagramDialog *ui;

    Trace *trace;

    std::vector<std::vector<double>> eyeBuffer[2];
    std::vector<std::vector<double>> *workingBuffer;
    std::vector<std::vector<double>> *finishedBuffer;

    Math::TDR *tdr;

    bool updating;
    bool firstUpdate;
};

#endif // EYEDIAGRAMDIALOG_H
