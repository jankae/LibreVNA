#include "sourcecaldialog.h"

#include <QDebug>

SourceCalDialog::SourceCalDialog(Device *dev)
    : AmplitudeCalDialog(dev)
{
    setWindowTitle("Source Calibration Dialog");
    LoadFromDevice();
}

void SourceCalDialog::SelectedPoint(double frequency, bool port2)
{
    Protocol::PacketInfo p;
    p.type = Protocol::PacketType::Generator;

    p.generator.frequency = frequency;
    p.generator.cdbm_level = excitationAmplitude * 100.0;
    if(frequency > 0) {
        if(port2) {
            p.generator.activePort = 2;
        } else {
            p.generator.activePort = 1;
        }
    } else {
        // invalid frequency, disable both ports
        p.generator.activePort = 0;
    }
    p.generator.applyAmplitudeCorrection = 0;
    dev->SendPacket(p);
}

void SourceCalDialog::AmplitudeChanged(AmplitudeCalDialog::CorrectionPoint &point, bool port2)
{
    auto *factor = port2 ? &point.correctionPort2 : &point.correctionPort1;
    const auto *amplitude = port2 ? &point.amplitudePort2 : &point.amplitudePort1;
    // calculate correction factor by comparing expected with measured amplitude
    *factor = (excitationAmplitude - *amplitude) * 100.0;
}

void SourceCalDialog::UpdateAmplitude(AmplitudeCalDialog::CorrectionPoint &point)
{
    point.amplitudePort1 = excitationAmplitude - (double) point.correctionPort1 / 100.0;
    point.amplitudePort2 = excitationAmplitude - (double) point.correctionPort2 / 100.0;
    point.port1set = true;
    point.port2set = true;
}
