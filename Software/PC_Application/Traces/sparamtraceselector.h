#ifndef SPARAMTRACESELECTOR_H
#define SPARAMTRACESELECTOR_H

#include <QWidget>
#include "tracemodel.h"
#include <QComboBox>

class SparamTraceSelector : public QWidget
{
    Q_OBJECT

public:
    SparamTraceSelector(const TraceModel &model, unsigned int num_ports, bool empty_allowed = false, std::set<unsigned int> skip = {});

    bool isValid();

    std::vector<Trace*> getTraces();
    unsigned int getPoints() { return points;};

signals:
    void selectionValid(bool valid);

private:
    void setInitialChoices();
    void traceSelectionChanged(QComboBox *cb);

    const TraceModel &model;
    std::vector<QComboBox*> boxes;
    unsigned int num_ports;
    bool empty_allowed;

    unsigned int points;
    double minFreq, maxFreq;
    bool valid;
};

#endif // SPARAMTRACESELECTOR_H
