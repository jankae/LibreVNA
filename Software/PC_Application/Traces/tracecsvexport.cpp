#include "tracecsvexport.h"
#include "ui_tracecsvexport.h"
#include <QDialogButtonBox>
#include <QPushButton>
#include <QFileDialog>
#include "csv.h"

using namespace std;

TraceCSVExport::TraceCSVExport(TraceModel &traceModel, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::TraceCSVExport),
    model(traceModel.getTraces())
{
    ui->setupUi(this);
    ui->listView->setModel(&model);
    ui->buttonBox->button(QDialogButtonBox::Ok)->setEnabled(false);
    connect(&model, &TraceCSVModel::selectionChanged, ui->buttonBox->button(QDialogButtonBox::Ok), &QPushButton::setEnabled);
}

TraceCSVExport::~TraceCSVExport()
{
    delete ui;
}

void TraceCSVExport::on_buttonBox_accepted()
{
    auto traces = model.tracesToExport();
    if(traces.size() == 0) {
        return;
    }

    auto filename = QFileDialog::getSaveFileName(nullptr, "Save calibration data", "", "CSV files (*.csv)", nullptr, QFileDialog::DontUseNativeDialog);
    if(filename.isEmpty()) {
        // aborted selection
        return;
    }
    if(!filename.endsWith(".csv")) {
        filename.append(".csv");
    }

    CSV csv;
    // create the first column (X data)
    vector<double> X;
    QString Xname = traces[0]->outputType() == Trace::DataType::Frequency ? "Frequency" : "Time";
    auto samples = traces[0]->numSamples();
    for(unsigned int i=0;i<samples;i++) {
        X.push_back(traces[0]->sample(i).x);
    }
    csv.addColumn(Xname, X);
    // add the trace data
    for(auto t : traces) {
        vector<double> real;
        vector<double> imag;
        auto samples = t->numSamples();
        for(unsigned int i=0;i<samples;i++) {
            real.push_back(t->sample(i).y.real());
            imag.push_back(t->sample(i).y.imag());
        }
        // check if this is a real or complex trace
        bool allZeros = std::all_of(imag.begin(), imag.end(), [](double i) { return i==0.0; });
        if(allZeros) {
            // only real values, one column is enough
            csv.addColumn(t->name(), real);
        } else {
            // complex values, need two columns
            csv.addColumn(t->name()+"_real", real);
            csv.addColumn(t->name()+"_imag", imag);
        }
    }

    csv.toFile(filename);
}

TraceCSVModel::TraceCSVModel(std::vector<Trace *> traces, QObject *parent)
    : QAbstractListModel(parent)
{
    this->traces = traces;
    save.resize(traces.size(), false);
    enabled.resize(traces.size(), true);
    points = 0;
    updateEnabledTraces();
}

int TraceCSVModel::rowCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent);
    return traces.size();
}

QVariant TraceCSVModel::data(const QModelIndex &index, int role) const
{
    if(!index.isValid()) {
        return QVariant();
    }
    switch(role) {
    case Qt::CheckStateRole:
        if(save[index.row()]) {
            return Qt::Checked;
        } else {
            return Qt::Unchecked;
        }
    case Qt::DisplayRole:
        return traces[index.row()]->name();
    default:
        return QVariant();
    }
}

bool TraceCSVModel::setData(const QModelIndex &index, const QVariant &value, int role)
{
    if(!index.isValid()) {
        return false;
    }
    if(role==Qt::CheckStateRole) {
        save[index.row()] = !save[index.row()];
        updateEnabledTraces();
        return true;
    } else {
        return QAbstractItemModel::setData(index, value, role);
    }
}

Qt::ItemFlags TraceCSVModel::flags(const QModelIndex &index) const
{
    unsigned int flags = Qt::ItemIsUserCheckable;
    if(index.isValid()) {
        if(enabled[index.row()]) {
            flags |= Qt::ItemIsEnabled;
        }
    }
    return (Qt::ItemFlags) flags;
}

std::vector<Trace *> TraceCSVModel::tracesToExport()
{
    vector<Trace*> ret;
    for(unsigned int i=0;i<traces.size();i++) {
        if(save[i]) {
            ret.push_back(traces[i]);
        }
    }
    return ret;
}

void TraceCSVModel::updateEnabledTraces()
{
    beginResetModel();
    // find first selected trace and use its settings
    points = 0;
    type = Trace::DataType::Invalid;
    for(unsigned int i=0;i<traces.size();i++) {
        if(save[i]) {
            points = traces[i]->numSamples();
            minX = traces[i]->minX();
            maxX = traces[i]->maxX();
            type = traces[i]->outputType();
        }
    }
    // second pass: compare to the settings and only enable identical traces
    for(unsigned int i=0;i<traces.size();i++) {
        auto enableTrace = true;
        if(traces[i]->numSamples() == 0 || traces[i]->outputType() == Trace::DataType::Invalid) {
            // trace has no valid data, unable to export
            enableTrace = false;
        }
        if(points > 0 && type != Trace::DataType::Invalid) {
            // check if this trace matches the already selected one
            if((traces[i]->numSamples() != points)
                    || (traces[i]->minX() != minX)
                    || (traces[i]->maxX() != maxX)
                    || (traces[i]->outputType() != type)) {
                // different settings, not possible to export in this combination
                enableTrace = false;
            }
        }
        enabled[i] = enableTrace;
        if(!enableTrace) {
            save[i] = false;
        }
    }
    endResetModel();
    emit selectionChanged(points > 0);
}
