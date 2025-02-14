#include "tracesetselector.h"

#include <QGridLayout>
#include <QLabel>

TraceSetSelector::TraceSetSelector(QWidget *parent) :
    QWidget(parent),
    model(nullptr)
{
    points = 0;
    ports = 0;
    lowerFreq = upperFreq = 0;
    referenceImpedance = 50.0;
    freqsSet = false;
    partialSelectionAllowed = false;

    setLayout(new QGridLayout);
    setPorts(2);
}

TraceSetSelector::~TraceSetSelector()
{
    blockSignals(true);
    // this will free all created widgets
    setPorts(0);
    blockSignals(false);
}

unsigned int TraceSetSelector::getPorts() const
{
    return ports;
}

void TraceSetSelector::setPorts(unsigned int newPorts)
{
    if(!model) {
        return;
    }
    QGridLayout *layout = static_cast<QGridLayout*>(this->layout());
    if(newPorts != ports) {
        ports = newPorts;
        // remove the previous widgets
        QLayoutItem *child;
        while ((child = layout->takeAt(0)) != 0)  {
            delete child->widget();
            delete child;
        }
        cTraces.clear();
        for(unsigned int i=0;i<ports;i++) {
            cTraces.push_back(std::vector<QComboBox*>());
            for(unsigned int j=0;j<ports;j++) {
                auto l = new QLabel("S"+QString::number(i+1)+QString::number(j+1)+":");
                auto c = new QComboBox();
                cTraces[i].push_back(c);
                connect(c, qOverload<int>(&QComboBox::currentIndexChanged), [=](int) {
                   selectionChanged(c);
                });
                layout->addWidget(l, i, j*2);
                layout->addWidget(c, i, j*2 + 1);
            }
        }
    }
    auto availableTraces = model->getTraces();
    for(unsigned int i=0;i<ports;i++) {
        for(unsigned int j=0;j<ports;j++) {
            auto c = cTraces[i][j];
            c->blockSignals(true);
            c->clear();
            // create possible trace selections
            c->addItem("None");
            for(auto t : availableTraces) {
                if(t->outputType() != Trace::DataType::Frequency) {
                    // can only add frequency traces
                    continue;
                }
                if(i == j && !t->isReflection()) {
                    // can not add through measurement at reflection port
                    continue;
                } else if(i != j && t->isReflection()) {
                    // can not add reflection measurement at through port
                    continue;
                }
                c->addItem(t->name(), QVariant::fromValue<Trace*>(t));
            }
            c->blockSignals(false);
        }
    }
    emit selectionChanged();
}

void TraceSetSelector::setDefaultTraces()
{
    auto traces = model->getTraces();
    for(unsigned int i=1;i<=ports;i++) {
        for(unsigned int j=1;j<=ports;j++) {
            QString defaultTraceName = "S"+QString::number(i)+QString::number(j);
            for(auto t : traces) {
                if(t->name() == defaultTraceName) {
                    setTrace(i, j, t);
                    break;
                }
            }
        }
    }
}

bool TraceSetSelector::setTrace(unsigned int destPort, unsigned int srcPort, Trace *t)
{
    if(destPort < 1 || destPort > ports || srcPort < 1 || srcPort > ports) {
        // invalid port selection
        return false;
    }
    auto c = cTraces[destPort-1][srcPort-1];
    // find required trace
    for(unsigned int i=1;i<(unsigned int) c->count();i++) {
        if(qvariant_cast<Trace*>(c->itemData(i)) == t) {
            c->setCurrentIndex(i);
            return true;
        }
    }
    // requested trace not found, unable to set
    return false;
}

Trace *TraceSetSelector::getTrace(unsigned int destPort, unsigned int srcPort)
{
    if(destPort < 1 || destPort > ports || srcPort < 1 || srcPort > ports) {
        // invalid port selection
        return nullptr;
    }
    auto c = cTraces[destPort-1][srcPort-1];
    if(c->currentIndex() == 0) {
        // no trace selected
        return nullptr;
    } else {
        return qvariant_cast<Trace*>(c->itemData(c->currentIndex()));
    }
}

bool TraceSetSelector::selectionValid()
{
    for(unsigned int i=1;i<=ports;i++) {
        for(unsigned int j=1;j<=ports;j++) {
            auto t = getTrace(i, j);
            if(!t && !partialSelectionAllowed) {
                // at least one trace is missing
                return false;
            }
            if(t && partialSelectionAllowed) {
                // at least one trace is present
                return true;
            }
        }
    }
    // if we get here, either:
    // - all traces are valid and partial selection is not allowed -> whole selection valid
    // or:
    // - all traces are invalid and partial selection is allowed -> whole selection invalid
    return !partialSelectionAllowed;
}

void TraceSetSelector::selectionChanged(QComboBox *c)
{
    if(c->currentIndex() != 0 && !freqsSet) {
        // the first trace has been selected, extract frequency info
        Trace *t = qvariant_cast<Trace*>(c->itemData(c->currentIndex()));
        points = t->size();
        referenceImpedance = t->getReferenceImpedance();
        if(points > 0) {
            lowerFreq = t->minX();
            upperFreq = t->maxX();
        }
        freqsSet = true;
        // remove all trace options with incompatible frequencies
        for(auto v1 : cTraces) {
            for(auto c : v1) {
                for(int i=1;i<c->count();i++) {
                    Trace *t = qvariant_cast<Trace*>(c->itemData(i));
                    if(t->getReferenceImpedance() != referenceImpedance || t->size() != points || (points > 0 && (t->minX() != lowerFreq || t->maxX() != upperFreq))) {
                        // this trace is not available anymore
                        c->removeItem(i);
                        // decrement to check the next index in the next loop iteration
                        i--;
                    }
                }
            }
        }
    } else if(c->currentIndex() == 0 && freqsSet) {
        // Check if all trace selections are set for none
        for(auto v1 : cTraces) {
            for(auto c : v1) {
                if(c->currentIndex() != 0) {
                    // some trace is still selected, abort
                    emit selectionChanged();
                    return;
                }
            }
        }
        // all traces set for none
        freqsSet = false;
        setPorts(ports);
    }
    emit selectionChanged();
}

double TraceSetSelector::getReferenceImpedance() const
{
    return referenceImpedance;
}

double TraceSetSelector::getUpperFreq() const
{
    return upperFreq;
}

double TraceSetSelector::getLowerFreq() const
{
    return lowerFreq;
}

unsigned int TraceSetSelector::getPoints() const
{
    return points;
}

void TraceSetSelector::setModel(TraceModel *newModel)
{
    model = newModel;
}

void TraceSetSelector::setPartialSelectionAllowed(bool newPartialSelectionAllowed)
{
    partialSelectionAllowed = newPartialSelectionAllowed;
}
