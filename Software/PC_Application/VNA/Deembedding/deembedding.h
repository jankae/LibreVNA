#ifndef DEEMBEDDING_H
#define DEEMBEDDING_H

#include "deembeddingoption.h"
#include "savable.h"
#include "Traces/tracemodel.h"

#include <vector>
#include <map>

#include <QObject>
#include <QDialog>
#include <QComboBox>

class Ui_DeembeddingMeasurementDialog;

class Deembedding : public QObject, public Savable
{
    Q_OBJECT
public:
    Deembedding(TraceModel &tm);
    ~Deembedding(){}

    void Deembed(VirtualDevice::VNAMeasurement &d);
    void Deembed(std::map<QString, Trace*> traceSet);

    void removeOption(unsigned int index);
    void addOption(DeembeddingOption* option);
    void swapOptions(unsigned int index);

    std::set<int> getAffectedPorts();

    std::vector<DeembeddingOption*>& getOptions() {return options;}
    nlohmann::json toJSON() override;
    void fromJSON(nlohmann::json j) override;
public slots:
    void configure();
signals:
    void triggerMeasurement(bool S11 = true, bool S12 = true, bool S21 = true, bool S22 = true);
    void optionAdded();
    void allOptionsCleared();
private:
    void measurementCompleted();
    void startMeasurementDialog(DeembeddingOption *option);
    std::vector<DeembeddingOption*> options;
    DeembeddingOption *measuringOption;
    TraceModel &tm;

    bool measuring;
    std::vector<VirtualDevice::VNAMeasurement> measurements;
    QDialog *measurementDialog;
    Ui_DeembeddingMeasurementDialog *measurementUI;

    unsigned long sweepPoints;

};

#endif // DEEMBEDDING_H
