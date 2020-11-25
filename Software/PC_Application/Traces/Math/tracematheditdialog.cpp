#include "tracematheditdialog.h"
#include "ui_tracematheditdialog.h"

TraceMathEditDialog::TraceMathEditDialog(Trace &t, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::TraceMathEditDialog)
{
    auto model = new MathModel(t);
    ui->setupUi(this);
    ui->view->setModel(model);
}

TraceMathEditDialog::~TraceMathEditDialog()
{
    delete ui;
}

MathModel::MathModel(Trace &t, QObject *parent)
    : QAbstractTableModel(parent),
      t(t)
{

}

int MathModel::rowCount(const QModelIndex &parent) const
{
    return t.getMath().size();
}

int MathModel::columnCount(const QModelIndex &parent) const
{
    return ColIndexLast;
}

QVariant MathModel::data(const QModelIndex &index, int role) const
{
    if(!index.isValid()) {
        return QVariant();
    }
    auto math = t.getMath().at(index.row());
    switch(index.column()) {
    case ColIndexEnabled:
        if(role == Qt::CheckStateRole) {
            return math.enabled ? Qt::Checked : Qt::Unchecked;
        }
        break;
    case ColIndexDescription:
        if(role == Qt::DisplayRole) {
            return math.math->description();
        }
        break;
    }
    return QVariant();
}

QVariant MathModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if(orientation == Qt::Horizontal && role == Qt::DisplayRole) {
        switch(section) {
        case ColIndexEnabled: return "Enabled"; break;
        case ColIndexDescription: return "Description"; break;
        default: break;
        }
    }
    return QVariant();
}

Qt::ItemFlags MathModel::flags(const QModelIndex &index) const
{
    int flags = Qt::NoItemFlags;
    if(index.row() > 1) {
        // the first entry is always the trace itself and not enabled
        flags |= Qt::ItemIsEnabled;
    }
    switch(index.column()) {
    case ColIndexEnabled: flags |= Qt::ItemIsUserCheckable; break;
    default:
        break;
    }
    return (Qt::ItemFlags) flags;
}
