#ifndef TRACESETSELECTOR_H
#define TRACESETSELECTOR_H

#include "Traces/tracemodel.h"

#include <QWidget>
#include <QComboBox>

namespace Ui {
class TraceSetSelector;
}

class TraceSetSelector : public QWidget
{
    Q_OBJECT

public:
    explicit TraceSetSelector(QWidget *parent = nullptr);
    ~TraceSetSelector();

    unsigned int getPorts() const;
    void setPorts(unsigned int newPorts);
    void setDefaultTraces();

    bool setTrace(unsigned int destPort, unsigned int srcPort, Trace *t);
    Trace *getTrace(unsigned int destPort, unsigned int srcPort);

    bool selectionValid();

    void setPartialSelectionAllowed(bool newPartialSelectionAllowed);

    void setModel(TraceModel *newModel);

    unsigned int getPoints() const;
    double getLowerFreq() const;
    double getUpperFreq() const;
    double getReferenceImpedance() const;
    TraceModel* getModel() const {return model;}

signals:
    void selectionChanged();

private:
    void selectionChanged(QComboBox *c);
    Ui::TraceSetSelector *ui;
    TraceModel *model;
    unsigned int ports;

    std::vector<std::vector<QComboBox*>> cTraces;

    unsigned int points;
    double lowerFreq, upperFreq;
    double referenceImpedance;
    bool freqsSet;

    bool partialSelectionAllowed;
};

#endif // TRACESETSELECTOR_H
