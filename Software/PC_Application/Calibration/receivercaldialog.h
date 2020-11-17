#ifndef RECEIVERCALDIALOG_H
#define RECEIVERCALDIALOG_H

#include "amplitudecaldialog.h"

class ReceiverCalDialog : public AmplitudeCalDialog
{
    Q_OBJECT
public:
    ReceiverCalDialog(Device *dev);
protected:
    Protocol::PacketType requestCommand() override { return Protocol::PacketType::RequestReceiverCal; }
    Protocol::PacketType pointType() override { return Protocol::PacketType::ReceiverCalPoint; }
    void SelectedPoint(double frequency, bool port2) override;
    void AmplitudeChanged(CorrectionPoint &point, bool port2) override;
    void UpdateAmplitude(CorrectionPoint& point) override;
private:
    static constexpr double excitationAmplitude = -20.0;
    double port1_result, port2_result; // raw (uncorrected) measurements from device
};

#endif // RECEIVERCALDIALOG_H
