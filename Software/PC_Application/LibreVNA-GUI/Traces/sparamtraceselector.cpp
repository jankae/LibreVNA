#include "sparamtraceselector.h"

#include <QFormLayout>
#include <QLabel>

using namespace std;

SparamTraceSelector::SparamTraceSelector(const TraceModel &model, std::vector<unsigned int> used_ports, bool empty_allowed)
    : model(model),
      empty_allowed(empty_allowed),
      used_ports(used_ports)
{
    createGUI();
    setInitialChoices();
}

SparamTraceSelector::SparamTraceSelector(const TraceModel &model, std::set<unsigned int> used_ports, bool empty_allowed)
    : model(model),
      empty_allowed(empty_allowed)
{
    // create vector from set
    std::copy(used_ports.begin(), used_ports.end(), std::back_inserter(this->used_ports));
    createGUI();
    setInitialChoices();
}

std::map<QString, Trace*> SparamTraceSelector::getTraces()
{
    std::map<QString, Trace*> ret;
    for(unsigned int i=0;i<used_ports.size();i++) {
        for(unsigned int j=0;j<used_ports.size();j++) {
            auto b = boxes[i*used_ports.size()+j];
            Trace *t;
            if(b->currentIndex() == 0) {
                t = nullptr;
            } else {
                t = qvariant_cast<Trace*>(b->itemData(b->currentIndex()));
            }
            QString name = "S"+QString::number(used_ports[i])+QString::number(used_ports[j]);
            ret[name] = t;
        }
    }
    return ret;
}

void SparamTraceSelector::setInitialChoices()
{
    for(unsigned int i=0;i<used_ports.size()*used_ports.size();i++) {
        boxes[i]->blockSignals(true);
        boxes[i]->clear();
        boxes[i]->addItem("None");
        for(auto t : model.getTraces()) {
            if(t->outputType() != Trace::DataType::Frequency) {
                // only frequency domain traces allowed
                continue;
            }
            if(t->size() == 0) {
                // can't select empty traces
                continue;
            }
            bool reflectionRequired = i%(used_ports.size()+1) == 0 ? true : false;
            if(reflectionRequired != t->isReflection()) {
                // invalid S parameter
                continue;
            }
            boxes[i]->addItem(t->name(), QVariant::fromValue<Trace*>(t));
        }
        boxes[i]->blockSignals(false);
    }
    minFreq = maxFreq = 0;
    points = 0;
    valid = empty_allowed;
}

void SparamTraceSelector::traceSelectionChanged(QComboBox *cb)
{
    // update available traces in combo boxes
    if(cb->currentIndex() != 0 && points == 0) {
        // the first trace has been selected, extract frequency info
        Trace *t = qvariant_cast<Trace*>(cb->itemData(cb->currentIndex()));
        points = t->size();
        if(points > 0) {
            minFreq = t->minX();
            maxFreq = t->maxX();
        }
        // remove all trace options with incompatible frequencies
        for(auto c : boxes) {
            if(!c->isVisible()) {
                // skip invisible selections
                continue;
            }
            for(int i=1;i<c->count();i++) {
                Trace *t = qvariant_cast<Trace*>(c->itemData(i));
                if(t->size() != points || (points > 0 && (t->minX() != minFreq || t->maxX() != maxFreq))) {
                    // this trace is not available anymore
                    c->removeItem(i);
                    // decrement to check the next index in the next loop iteration
                    i--;
                }
            }
        }
        // try to find matching traces based on their names (convenient autoselection of matching traces)
        auto text = cb->currentText();
        text.chop(2);
        if(text.endsWith("S")) {
            // tracename ended in Sxx, probably other traces with matching prefix available
            for(unsigned int i=0;i<used_ports.size()*used_ports.size();i++) {
                auto b = boxes[i];
                if(b == cb) {
                    // skip this box
                    continue;
                }
                for(int j=0;j<b->count();j++) {
                    auto candidate = b->itemText(j);
                    // check if correct parameter
                    QString expectedSparam = QString::number(i/used_ports.size()+1)+QString::number(i%used_ports.size()+1);
                    if(!candidate.endsWith(expectedSparam)) {
                        // wrong S parameter, skip
                        continue;
                    }
                    candidate.chop(2);
                    if(candidate != text) {
                        // trace name does not match
                        continue;
                    }
                    // possible match, select for this box
                    b->setCurrentIndex(j);
                    break;
                }
            }
        }

    } else if(cb->currentIndex() == 0 && points > 0) {
        emit selectionValid(false);
        // Check if all trace selections are set for none
        for(auto c : boxes) {
            if(!c->isVisible()) {
                // skip invisible selections
                continue;
            }
            if(c->currentIndex() != 0) {
                // some trace is still selected, abort
                return;
            }
        }
        // all traces set for none
        points = 0;
        minFreq = 0;
        maxFreq = 0;
        setInitialChoices();
    }
    if(empty_allowed) {
        // always valid
        emit selectionValid(true);
    } else {
        // actually need to check
        valid = true;
        for(auto c : boxes) {
            if(!c->isVisible()) {
                // skip invisible selections
                continue;
            }
            if (c->currentIndex() == 0) {
                valid = false;
                break;
            }
        }
        emit selectionValid(valid);
    }
}

void SparamTraceSelector::createGUI()
{
    // Create comboboxes
    auto layout = new QFormLayout;
    setLayout(layout);
    for(unsigned int i=0;i<used_ports.size();i++) {
        for(unsigned int j=0;j<used_ports.size();j++) {
            auto label = new QLabel("S"+QString::number(used_ports[i])+QString::number(used_ports[j])+":");
            auto box = new QComboBox();
            connect(box, qOverload<int>(&QComboBox::currentIndexChanged), [=](int) {
               traceSelectionChanged(box);
            });
            boxes.push_back(box);
            layout->addRow(label, box);
        }
    }
}
