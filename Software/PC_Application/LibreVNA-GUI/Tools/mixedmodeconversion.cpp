#include "mixedmodeconversion.h"
#include "ui_mixedmodeconversion.h"

#include <QPushButton>

MixedModeConversion::MixedModeConversion(TraceModel &m, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::MixedModeConversion)
{
    setAttribute(Qt::WA_DeleteOnClose);
    ui->setupUi(this);
    ui->selector->setModel(&m);
    ui->selector->setPorts(4);
    connect(ui->selector, qOverload<>(&TraceSetSelector::selectionChanged), this, &MixedModeConversion::selectionChanged);
    connect(ui->prefix, &QLineEdit::textChanged, this, &MixedModeConversion::selectionChanged);
    connect(ui->buttonBox->button(QDialogButtonBox::Ok), &QPushButton::clicked, this, [=](){
        emit tracesCreated(traces);
        accept();
    });
    connect(ui->buttonBox->button(QDialogButtonBox::Cancel), &QPushButton::clicked, this, &QDialog::reject);
    selectionChanged();
    ui->selector->setDefaultTraces();
}

MixedModeConversion::~MixedModeConversion()
{
    delete ui;
}

void MixedModeConversion::selectionChanged()
{
    for(auto t : traces) {
        delete t;
    }
    traces.clear();
    class Source {
    public:
        Source(TraceSetSelector *sel, QString name) {
            this->name = name;
            t = sel->getTrace(name.mid(1, 1).toUInt(), name.mid(2, 1).toUInt());
        }
        QString name;
        Trace *t;
    };

    std::vector<Source> sources = {
        Source(ui->selector, "S11"),
        Source(ui->selector, "S12"),
        Source(ui->selector, "S13"),
        Source(ui->selector, "S14"),

        Source(ui->selector, "S21"),
        Source(ui->selector, "S22"),
        Source(ui->selector, "S23"),
        Source(ui->selector, "S24"),

        Source(ui->selector, "S31"),
        Source(ui->selector, "S32"),
        Source(ui->selector, "S33"),
        Source(ui->selector, "S34"),

        Source(ui->selector, "S41"),
        Source(ui->selector, "S42"),
        Source(ui->selector, "S43"),
        Source(ui->selector, "S44"),
    };

    class Destination {
    public:
        Destination(std::vector<Source> sources, QString name, QString formula, double reference_impedance)
        {
            this->name = name;
            t = new Trace(name);
            t->fromMath();
            t->setMathFormula(formula);
            t->setReferenceImpedance(reference_impedance);
            // is a reflection trace if the last two chars in the name are the same
            t->setReflection(name[name.size()-1] == name[name.size()-2]);
            // add math sources
            int index = 0;
            while((index = formula.indexOf("S", index)) != -1) {
                QString source = formula.mid(index, 3);
                // find the source trace
                bool sourceSet = false;
                for(auto s : sources) {
                    if(!s.t) {
                        // not set, unable to use this source
                        continue;
                    }
                    if(s.name == source) {
                        if(t->addMathSource(s.t, s.name)) {
                            // added as source, exit loop
                            sourceSet = true;
                            break;
                        } else {
                            // failed to add
                            delete t;
                            t = nullptr;
                            return;
                        }
                    }
                }
                if(!sourceSet) {
                    // can't find this source
                    delete t;
                    t = nullptr;
                    return;
                }
                index++;
            }
        }
        QString name;
        Trace *t;
    };

    auto prefix = ui->prefix->text();

    std::vector<Destination> destinations = {
        Destination(sources, prefix+"SDD11", "0.5*(S11-S13-S31+S33)", 2*ui->selector->getReferenceImpedance()),
        Destination(sources, prefix+"SDD12", "0.5*(S12-S14-S32+S34)", 2*ui->selector->getReferenceImpedance()),
        Destination(sources, prefix+"SDD21", "0.5*(S21-S23-S41+S43)", 2*ui->selector->getReferenceImpedance()),
        Destination(sources, prefix+"SDD22", "0.5*(S22-S24-S42+S44)", 2*ui->selector->getReferenceImpedance()),

        Destination(sources, prefix+"SDC11", "0.5*(S11+S13-S31-S33)", 0.5*ui->selector->getReferenceImpedance()),
        Destination(sources, prefix+"SDC12", "0.5*(S12+S14-S32-S34)", 0.5*ui->selector->getReferenceImpedance()),
        Destination(sources, prefix+"SDC21", "0.5*(S21+S23-S41-S43)", 0.5*ui->selector->getReferenceImpedance()),
        Destination(sources, prefix+"SDC22", "0.5*(S22+S24-S42-S44)", 0.5*ui->selector->getReferenceImpedance()),

        Destination(sources, prefix+"SCD11", "0.5*(S11-S13+S31-S33)", 2*ui->selector->getReferenceImpedance()),
        Destination(sources, prefix+"SCD12", "0.5*(S12-S14+S32-S34)", 2*ui->selector->getReferenceImpedance()),
        Destination(sources, prefix+"SCD21", "0.5*(S21-S23+S41-S43)", 2*ui->selector->getReferenceImpedance()),
        Destination(sources, prefix+"SCD22", "0.5*(S22-S24+S42-S44)", 2*ui->selector->getReferenceImpedance()),

        Destination(sources, prefix+"SCC11", "0.5*(S11+S13+S31+S33)", 0.5*ui->selector->getReferenceImpedance()),
        Destination(sources, prefix+"SCC12", "0.5*(S12+S14+S32+S34)", 0.5*ui->selector->getReferenceImpedance()),
        Destination(sources, prefix+"SCC21", "0.5*(S21+S23+S41+S43)", 0.5*ui->selector->getReferenceImpedance()),
        Destination(sources, prefix+"SCC22", "0.5*(S22+S24+S42+S44)", 0.5*ui->selector->getReferenceImpedance()),
    };

    ui->list->clear();
    for(auto d : destinations) {
        if(d.t) {
            traces.push_back(d.t);
            ui->list->addItem(d.name);
        }
    }
    ui->buttonBox->button(QDialogButtonBox::Ok)->setEnabled(traces.size() > 0);
}
