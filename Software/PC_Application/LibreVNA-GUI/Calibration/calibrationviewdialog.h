#ifndef CALIBRATIONVIEWDIALOG_H
#define CALIBRATIONVIEWDIALOG_H

#include <QDialog>

#include "calibration.h"
#include <QGraphicsScene>

namespace Ui {
class CalibrationViewDialog;
}

class CalibrationViewDialog : public QDialog
{
    Q_OBJECT

public:
    explicit CalibrationViewDialog(Calibration *cal, unsigned int ports, QWidget *parent = nullptr);
    ~CalibrationViewDialog();

private slots:
    void populateScene();
private:
    static constexpr int marginTop = 10;
    static constexpr int marginBottom = 10;
    static constexpr int marginLeft = 30;
    static constexpr int marginRight = 10;

    static constexpr int portHeight = 170;
    static constexpr int portForwardYOffset = -50;
    static constexpr int portReverseYOffset = 50;
    static constexpr int boxSpacing = portReverseYOffset - portForwardYOffset;

    static constexpr int portSize = 10;
    static constexpr int arrowLength = 15;
    static constexpr int arrowWidth = 10;
    static constexpr int junctionSize = 6;
    static constexpr int labelDistance = 6;

    static constexpr int pathSpacing = 40;

    static const QColor colorNoCal;
    static const QColor colorHasCal;

    Ui::CalibrationViewDialog *ui;
    Calibration *cal;
    unsigned int ports;
    QGraphicsScene *scene;
};

#endif // CALIBRATIONVIEWDIALOG_H
