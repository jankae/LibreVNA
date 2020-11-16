#include "sourcecaldialog.h"

SourceCalDialog::SourceCalDialog(Device *dev)
    : AmplitudeCalDialog(dev)
{
    setWindowTitle("Source Calibration Dialog");
    LoadFromDevice();
}
