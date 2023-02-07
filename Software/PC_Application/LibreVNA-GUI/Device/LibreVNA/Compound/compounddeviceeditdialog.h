#ifndef COMPOUNDDEVICEEDITDIALOG_H
#define COMPOUNDDEVICEEDITDIALOG_H

#include "compounddevice.h"

#include <QDialog>
#include <QLabel>
#include <QComboBox>

namespace Ui {
class CompoundDeviceEditDialog;
}

class DeviceFrame : public QFrame {
    Q_OBJECT
public:
    DeviceFrame(CompoundDevice *dev, unsigned int position);
    ~DeviceFrame();
    void setPosition(int pos);
    void update();
    QString getSerial();
    int getPort1();
    int getPort2();
signals:
    void deleted();
    void settingChanged();
private:
    static constexpr int frameSize = 350;
    QComboBox *serial;
    QComboBox *port1, *port2;

    unsigned int position;
    CompoundDevice *dev;
};

class CompoundDeviceEditDialog : public QDialog
{
    Q_OBJECT

public:
    explicit CompoundDeviceEditDialog(CompoundDevice *cdev, QWidget *parent = nullptr);
    ~CompoundDeviceEditDialog();

private slots:
    void checkIfOkay();
private:
    static constexpr int frameSize = 350;
    void setInitialGUI();
    DeviceFrame *frameAtPosition(int pos);
    unsigned int findInsertPosition(int xcoord);
    void addFrameAtPosition(int pos, DeviceFrame *c);
    void addFrame(int index, DeviceFrame *c);
    void createDragFrame(DeviceFrame *c);
    void updateInsertIndicator(int xcoord);
    bool eventFilter(QObject *object, QEvent *event) override;

    void removeDeviceFrame(DeviceFrame *dev);

    void updateDeviceFrames();

    Ui::CompoundDeviceEditDialog *ui;
    CompoundDevice ldev;

    QWidget *graph, *insertIndicator;
    QPoint dragStartPosition;
    bool dropPending;
    DeviceFrame *dragFrame;
    DeviceFrame *dropFrame;

    std::vector<DeviceFrame*> deviceFrames;
};

#endif // COMPOUNDDEVICEEDITDIALOG_H
