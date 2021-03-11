#include "traceimportdialog.h"
#include "ui_traceimportdialog.h"
#include <QAbstractTableModel>
#include <QObject>
#include <QModelIndex>
#include <QColorDialog>

TraceImportDialog::TraceImportDialog(TraceModel &model, std::vector<Trace*> traces, QString prefix, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::TraceImportDialog),
    model(model)
{
    ui->setupUi(this);
    tableModel = new TraceParameterModel(traces, prefix);
    ui->tableView->setModel(tableModel);
}

TraceImportDialog::~TraceImportDialog()
{
    delete ui;
    delete tableModel;
}

void TraceImportDialog::on_buttonBox_accepted()
{
    auto traces = tableModel->import(model);
    emit importFinsished(traces);
}

TraceParameterModel::TraceParameterModel(std::vector<Trace *> traces, QString prefix, QObject *parent)
    : QAbstractTableModel(parent),
      traces(traces)
{
    int hue = 0;
    for(auto t : traces) {
        Parameter p;
        p.name = prefix + t->name();
        p.color = QColor::fromHsl((hue++ * 30) % 360, 250, 128);
        p.enabled = true;
        p.trace = t->name();
        params.push_back(p);
    }
}

int TraceParameterModel::rowCount(const QModelIndex &) const {
    return params.size();
}

int TraceParameterModel::columnCount(const QModelIndex &) const {
    return 4;
}

QVariant TraceParameterModel::data(const QModelIndex &index, int role) const {
    if (!index.isValid())
        return QVariant();

    if ((unsigned int) index.row() >= params.size())
        return QVariant();
    auto p = params[index.row()];
    if (index.column() == 0) {
        if (role == Qt::DisplayRole) {
            return p.trace;
        } else {
            return QVariant();
        }
    } else if (index.column() == 1) {
        if (role == Qt::CheckStateRole) {
            if(p.enabled) {
                return Qt::Checked;
            } else {
                return Qt::Unchecked;
            }
        } else {
            return QVariant();
        }
    } else if (index.column() == 2) {
        if (role == Qt::BackgroundRole || role == Qt::EditRole) {
            if(p.enabled) {
                return p.color;
            } else {
                return (QColor) Qt::gray;
            }
        } else {
            return QVariant();
        }
    } else if (index.column() == 3) {
        if (role == Qt::DisplayRole || role == Qt::EditRole) {
            return p.name;
        } else {
            return QVariant();
        }
    } else {
        return QVariant();
    }
}

QVariant TraceParameterModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if(orientation == Qt::Horizontal && role == Qt::DisplayRole) {
        switch(section) {
        case 0: return "Parameter"; break;
        case 1: return "Import"; break;
        case 2: return "Color"; break;
        case 3: return "Tracename"; break;
        default: return QVariant(); break;
        }
    } else {
        return QVariant();
    }
}

bool TraceParameterModel::setData(const QModelIndex &index, const QVariant &value, int role)
{
    if((unsigned int) index.row() >= params.size()) {
        return false;
    }
    auto &p = params[index.row()];
    if(role == Qt::CheckStateRole && index.column()==1) {
        if((Qt::CheckState)value.toInt() == Qt::Checked) {
            p.enabled = true;
        } else {
            p.enabled = false;
        }
        dataChanged(this->index(index.row(),2), this->index(index.row(),3));
        return true;
    } else if(role == Qt::EditRole && index.column() == 2) {
        p.color = value.value<QColor>();
        return true;
    } else if(role == Qt::EditRole && index.column() == 3) {
        p.name = value.toString();
        return true;
    }
    return false;
}

Qt::ItemFlags TraceParameterModel::flags(const QModelIndex &index) const
{
    int flags = Qt::NoItemFlags;
    if(index.column() == 0) {
        flags |= Qt::ItemIsEnabled;
    } else if(index.column() == 1) {
        flags |= Qt::ItemIsUserCheckable;
        flags |= Qt::ItemIsEnabled;
    } else if(index.column() == 2) {
        if(params[index.row()].enabled) {
            flags |= Qt::ItemIsEnabled;
        }
    } else if(index.column() == 3) {
        flags |= Qt::ItemIsEditable;
        if(params[index.row()].enabled) {
            flags |= Qt::ItemIsEnabled;
        }
    }
    return (Qt::ItemFlags) flags;
}

std::vector<Trace *> TraceParameterModel::import(TraceModel &model)
{
    std::vector<Trace*> importedTraces;
    for(unsigned int i=0;i<params.size();i++) {
        if(params[i].enabled) {
            traces[i]->setColor(params[i].color);
            traces[i]->setName(params[i].name);
            model.addTrace(traces[i]);
            importedTraces.push_back(traces[i]);
        } else {
            delete traces[i];
        }
    }
    return importedTraces;
}

void TraceImportDialog::on_tableView_doubleClicked(const QModelIndex &index)
{
    if(index.column() == 2 && tableModel->params[index.row()].enabled) {
        auto initialColor = tableModel->params[index.row()].color;
        auto newColor = QColorDialog::getColor(initialColor, this, "Select color", QColorDialog::DontUseNativeDialog);
        if(newColor.isValid()) {
            tableModel->setData(index, newColor);
        }
    }
}
