#ifndef DEEMBEDDING_H
#define DEEMBEDDING_H

#include "deembeddingoption.h"
#include <vector>
#include <QObject>
#include "savable.h"
#include "Traces/tracemodel.h"
#include <QDialog>
#include <QComboBox>

class Ui_DeembeddingMeasurementDialog;

class Deembedding : public QObject, public Savable
{
    Q_OBJECT
public:
    Deembedding(TraceModel &tm);
    ~Deembedding(){};

    void Deembed(Protocol::Datapoint &d);
    void Deembed(Trace &S11, Trace &S12, Trace &S21, Trace &S22);

    void removeOption(unsigned int index);
    void addOption(DeembeddingOption* option);
    void swapOptions(unsigned int index);
    std::vector<DeembeddingOption*>& getOptions() {return options;};
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
    void startMeasurementDialog(bool S11, bool S12, bool S21, bool S22);
    std::vector<DeembeddingOption*> options;
    DeembeddingOption *measuringOption;
    TraceModel &tm;

    bool measuring;
    std::vector<Protocol::Datapoint> measurements;
    QDialog *measurementDialog;
    Ui_DeembeddingMeasurementDialog *measurementUI;

    unsigned long sweepPoints;

};

#endif // DEEMBEDDING_H
