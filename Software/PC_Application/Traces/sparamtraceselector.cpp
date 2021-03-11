#include "sparamtraceselector.h"

#include <QFormLayout>
#include <QLabel>

using namespace std;

SparamTraceSelector::SparamTraceSelector(const TraceModel &model, unsigned int num_ports, bool empty_allowed, std::set<unsigned int> skip)
    : model(model),
      num_ports(num_ports),
      empty_allowed(empty_allowed)
{
    // Create comboboxes
    auto layout = new QFormLayout;
    setLayout(layout);
    for(unsigned int i=0;i<num_ports;i++) {
        for(unsigned int j=0;j<num_ports;j++) {
            auto label = new QLabel("S"+QString::number(i+1)+QString::number(j+1)+":");
            auto box = new QComboBox();
            connect(box, qOverload<int>(&QComboBox::currentIndexChanged), [=](int) {
               traceSelectionChanged(box);
            });
            boxes.push_back(box);
            layout->addRow(label, box);
            if(skip.count(i*num_ports + j)) {
                label->setVisible(false);
                box->setVisible(false);
            }
        }
    }

    setInitialChoices();
}

std::vector<Trace*> SparamTraceSelector::getTraces()
{
    vector<Trace*> ret;
    for(auto b : boxes) {
        if(b->currentIndex() == 0) {
            ret.push_back(nullptr);
        } else {
            auto trace = qvariant_cast<Trace*>(b->itemData(b->currentIndex()));
            ret.push_back(trace);
        }
    }
    return ret;
}

void SparamTraceSelector::setInitialChoices()
{
    for(unsigned int i=0;i<num_ports*num_ports;i++) {
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
            bool reflectionRequired = i%(num_ports+1) == 0 ? true : false;
            if(reflectionRequired != t->isReflection()) {
                // invalid S parameter
                continue;
            }
            boxes[i]->addItem(t->name(), QVariant::fromValue<Trace*>(t));
        }
        boxes[i]->blockSignals(false);
    }
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
            for(unsigned int i=0;i<num_ports*num_ports;i++) {
                auto b = boxes[i];
                if(b == cb) {
                    // skip this box
                    continue;
                }
                for(int j=0;j<b->count();j++) {
                    auto candidate = b->itemText(j);
                    // check if correct parameter
                    QString expectedSparam = QString::number(i/num_ports+1)+QString::number(i%num_ports+1);
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
