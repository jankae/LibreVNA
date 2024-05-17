#ifndef DEEMBEDDING_H
#define DEEMBEDDING_H

#include "deembeddingoption.h"
#include "savable.h"
#include "Traces/tracemodel.h"
#include "scpi.h"

#include <vector>
#include <map>

#include <QObject>
#include <QDialog>
#include <QComboBox>

class Ui_DeembeddingMeasurementDialog;

class Deembedding : public QObject, public Savable, public SCPINode
{
    Q_OBJECT
public:
    Deembedding(TraceModel &tm);
    ~Deembedding(){}

    void Deembed(DeviceDriver::VNAMeasurement &d);
    void Deembed(std::map<QString, Trace*> traceSet);

    void removeOption(unsigned int index);
    void addOption(DeembeddingOption* option);
    void swapOptions(unsigned int index);
    void clear();

    bool isMeasuring();

    std::set<unsigned int> getAffectedPorts();
    void setPointsInSweepForMeasurement(unsigned int points);

    std::vector<DeembeddingOption*>& getOptions() {return options;}
    nlohmann::json toJSON() override;
    void fromJSON(nlohmann::json j) override;
public slots:
    void configure();
signals:
    void triggerMeasurement();
    void finishedMeasurement();
    void optionAdded();
    void allOptionsCleared();
private:
    void measurementCompleted();
    void startMeasurementDialog(DeembeddingOption *option);
    void updateSCPINames();
    std::vector<DeembeddingOption*> options;
    DeembeddingOption *measuringOption;
    TraceModel &tm;

    bool measuring;
    std::vector<DeviceDriver::VNAMeasurement> measurements;
    QDialog *measurementDialog;
    Ui_DeembeddingMeasurementDialog *measurementUI;

    unsigned long sweepPoints;

};

#endif // DEEMBEDDING_H
