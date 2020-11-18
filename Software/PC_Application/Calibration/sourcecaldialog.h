#ifndef SOURCECALDIALOG_H
#define SOURCECALDIALOG_H

#include <QObject>
#include "amplitudecaldialog.h"

class SourceCalDialog : public AmplitudeCalDialog
{
    Q_OBJECT
public:
    SourceCalDialog(Device *dev);
protected:
    Protocol::PacketType requestCommand() override { return Protocol::PacketType::RequestSourceCal; }
    Protocol::PacketType pointType() override { return Protocol::PacketType::SourceCalPoint; }
    void SelectedPoint(double frequency, bool port2) override;
    void AmplitudeChanged(CorrectionPoint &point, bool port2) override;
    void UpdateAmplitude(CorrectionPoint& point) override;
};

#endif // SOURCECALDIALOG_H
