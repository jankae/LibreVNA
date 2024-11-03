#include "tracecsvexport.h"

#include "ui_tracecsvexport.h"
#include "csv.h"
#include "preferences.h"
#include "traceaxis.h"

#include <QDialogButtonBox>
#include <QPushButton>
#include <QFileDialog>


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
    connect(&model, &TraceCSVModel::selectionChanged, [&](){
        auto traces = model.tracesToExport();
        if(traces.size() == 0) {
            ui->listColumns->clear();
        } else if(ui->listColumns->count() == 0) {
            // first trace has been selected, fill column selection
            auto t = traces.front();
            auto domain = t->outputType();
            auto Xaxis = XAxis::Type::Last;
            switch(domain) {
            case Trace::DataType::Invalid:
            case Trace::DataType::Frequency: Xaxis = XAxis::Type::Frequency; break;
            case Trace::DataType::Power: Xaxis = XAxis::Type::Power; break;
            case Trace::DataType::Time: Xaxis = XAxis::Type::Time; break;
            case Trace::DataType::TimeZeroSpan: Xaxis = XAxis::Type::TimeZeroSpan; break;
            }
            if(Xaxis == XAxis::Type::Last) {
                // invalid axis selection
                return;
            }
            for(auto ytype : YAxis::getSupported(Xaxis, traceModel.getSource())) {
                if(ytype != YAxis::Type::Disabled) {
                    auto item = new QListWidgetItem(YAxis::TypeToName(ytype), ui->listColumns);
                    item->setCheckState(Qt::Unchecked);
                }
            }
            // first fill of selection, nothing selected yet, disable OK button
            ui->buttonBox->button(QDialogButtonBox::Ok)->setEnabled(false);
        }
    });
    connect(ui->listColumns, &QListWidget::itemChanged, this, &TraceCSVExport::columnSelectionChanged);
}

TraceCSVExport::~TraceCSVExport()
{
    delete ui;
}

void TraceCSVExport::columnSelectionChanged()
{
    auto types = getSelectedYAxisTypes();
    ui->buttonBox->button(QDialogButtonBox::Ok)->setEnabled(types.size() > 0);
}

void TraceCSVExport::on_buttonBox_accepted()
{
    auto traces = model.tracesToExport();
    if(traces.size() == 0) {
        return;
    }

    auto filename = QFileDialog::getSaveFileName(nullptr, "Save calibration data", "", "CSV files (*.csv)", nullptr, Preferences::QFileDialogOptions());
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
    QString Xname = Trace::dataTypeToString(traces[0]->outputType());
    auto samples = traces[0]->numSamples();
    for(unsigned int i=0;i<samples;i++) {
        X.push_back(traces[0]->sample(i).x);
    }
    csv.addColumn(Xname, X);
    // add the trace data
    for(auto trace : traces) {
        for(auto ytype : getSelectedYAxisTypes()) {
            auto axis = YAxis();
            axis.set(ytype, false, false, 0, 1, 10, false);
            auto samples = trace->numSamples();
            vector<double> values;
            for(unsigned int i=0;i<samples;i++) {
                values.push_back(axis.sampleToCoordinate(trace->sample(i), trace, i));
            }
            csv.addColumn(trace->name()+"_"+axis.TypeToName(), values);
        }
    }

    csv.toFile(filename);
}

std::vector<YAxis::Type> TraceCSVExport::getSelectedYAxisTypes()
{
    std::vector<YAxis::Type> ret;
    for(int i=0;i<ui->listColumns->count();i++) {
        auto item = ui->listColumns->item(i);
        if(item->checkState() == Qt::Checked) {
            auto type = YAxis::TypeFromName(item->text());
            ret.push_back(type);
        }
    }
    return ret;
}

TraceCSVModel::TraceCSVModel(std::vector<Trace *> traces, QObject *parent)
    : QAbstractListModel(parent)
{
    this->traces = traces;
    save.resize(traces.size(), false);
    enabled.resize(traces.size(), true);
    points = 0;
    minX = maxX = 0;
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
