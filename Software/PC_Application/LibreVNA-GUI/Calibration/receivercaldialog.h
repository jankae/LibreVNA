#ifndef RECEIVERCALDIALOG_H
#define RECEIVERCALDIALOG_H

#include "amplitudecaldialog.h"
#include "modehandler.h"

class ReceiverCalDialog : public AmplitudeCalDialog
{
    Q_OBJECT
public:
    ReceiverCalDialog(Device *dev, ModeHandler *handler);
protected:
    Protocol::PacketType requestCommand() override { return Protocol::PacketType::RequestReceiverCal; }
    Protocol::PacketType pointType() override { return Protocol::PacketType::ReceiverCalPoint; }
    void SelectedPoint(double frequency, bool port2) override;
    void AmplitudeChanged(CorrectionPoint &point, bool port2) override;
    void UpdateAmplitude(CorrectionPoint& point) override;
};

#endif // RECEIVERCALDIALOG_H
