#include "receivercaldialog.h"

ReceiverCalDialog::ReceiverCalDialog(Device *dev)
    : AmplitudeCalDialog(dev)
{
    setWindowTitle("Receiver Calibration Dialog");
    LoadFromDevice();
}

void ReceiverCalDialog::SelectedPoint(double frequency, bool)
{
    if(frequency > 0) {
        Protocol::PacketInfo p = {};
        p.type = Protocol::PacketType::SpectrumAnalyzerSettings;
        p.spectrumSettings.RBW = 10000;
        p.spectrumSettings.UseDFT = 0;
        // setup 3 points centered around the measurement frequency (zero span not supported yet)
        p.spectrumSettings.f_stop = frequency + 1.0;
        p.spectrumSettings.f_start = frequency - 1.0;
        p.spectrumSettings.pointNum = 3;
        p.spectrumSettings.Detector = 0;
        p.spectrumSettings.SignalID = 1;
        p.spectrumSettings.WindowType = 3;
        p.spectrumSettings.applyReceiverCorrection = 0;
        dev->SendPacket(p);
    } else {
        // invalid frequency, disable
        dev->SetIdle();
    }
}

void ReceiverCalDialog::AmplitudeChanged(AmplitudeCalDialog::CorrectionPoint &point, bool port2)
{
    auto m = averageMeasurement();
    auto *factor = port2 ? &point.correctionPort2 : &point.correctionPort1;
    const auto *amplitude = port2 ? &point.amplitudePort2 : &point.amplitudePort1;
    const auto *measured = port2 ? &m.port2 : &m.port1;
    // calculate correction factor by comparing expected with measured amplitude
    *factor = (*amplitude - *measured) * 100.0;
}

void ReceiverCalDialog::UpdateAmplitude(AmplitudeCalDialog::CorrectionPoint &point)
{
    // This point was just received from the device, it is not possible to know the actual amplitude because the
    // applied power level during the calibration is not saved (only the correction value). This is not a problem
    // because the correction value is still valid but the missing values look weird in the GUI
    // TODO change this?
    point.amplitudePort1 = std::numeric_limits<double>::quiet_NaN();
    point.amplitudePort2 = std::numeric_limits<double>::quiet_NaN();
    point.port1set = true;
    point.port2set = true;
}
