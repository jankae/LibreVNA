#ifndef EYEDIAGRAMDIALOG_H
#define EYEDIAGRAMDIALOG_H

#include "Traces/tracemodel.h"

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

private:
    void paintEvent(QPaintEvent *event) override;

    EyeDiagramDialog *dialog;
};

class EyeDiagramDialog : public QDialog
{
    Q_OBJECT

public:
    explicit EyeDiagramDialog(TraceModel &model);
    ~EyeDiagramDialog();

    double getIntensity(unsigned int x, unsigned int y);

public slots:
    bool triggerUpdate();
    bool update(unsigned int width, unsigned int height);

signals:
    void updateDone();

private:
signals:
    void updatePercent(int percent);

private:
    static constexpr double yOverrange = 0.2;
    void updateThread(unsigned int width, unsigned int height);

    Ui::EyeDiagramDialog *ui;

    Trace *trace;

    std::vector<std::vector<double>> eyeBuffer[2];
    std::vector<std::vector<double>> *workingBuffer;
    std::vector<std::vector<double>> *finishedBuffer;

    bool updating;
};

#endif // EYEDIAGRAMDIALOG_H
