#include "tracematheditdialog.h"
#include "ui_tracematheditdialog.h"
#include <QHeaderView>

#include "ui_newtracemathdialog.h"
namespace Ui {
class NewTraceMathDialog;
}

#include "medianfilter.h"

TraceMathEditDialog::TraceMathEditDialog(Trace &t, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::TraceMathEditDialog)
{
    auto model = new MathModel(t);
    ui->setupUi(this);
    ui->view->setModel(model);

    QHeaderView *headerView = ui->view->horizontalHeader();
    headerView->setSectionResizeMode(MathModel::ColIndexDescription, QHeaderView::Stretch);
    headerView->setSectionResizeMode(MathModel::ColIndexStatus, QHeaderView::ResizeToContents);
    headerView->setSectionResizeMode(MathModel::ColIndexDomain, QHeaderView::ResizeToContents);

    connect(ui->view->selectionModel(), &QItemSelectionModel::currentRowChanged, [=](const QModelIndex &current, const QModelIndex &previous){
        Q_UNUSED(previous)
        if(!current.isValid()) {
            ui->bDelete->setEnabled(false);
            ui->bMoveUp->setEnabled(false);
            ui->bMoveDown->setEnabled(false);
        } else {
            ui->bDelete->setEnabled(true);
            ui->bMoveUp->setEnabled(current.row() > 1);
            ui->bMoveDown->setEnabled(current.row() + 1 < model->rowCount());
        }
    });

    connect(ui->view, &QTableView::doubleClicked, [&](const QModelIndex &index) {
        if(index.isValid()) {
            auto math = t.getMathOperations().at(index.row()).math;
            math->edit();
        }
    });

    connect(ui->bAdd, &QPushButton::clicked, [=](){
        auto d = new QDialog();
        auto ui = new Ui::NewTraceMathDialog();
        ui->setupUi(d);
        for(int i = 0; i < (int) TraceMath::Type::Last;i++) {
            auto info = TraceMath::getInfo(static_cast<TraceMath::Type>(i));
            ui->list->addItem(info.name);
            if(!info.explanationWidget) {
                info.explanationWidget = new QWidget();
            }
            ui->stack->addWidget(info.explanationWidget);
        }
        // always show the widget for the selected function
        connect(ui->list, &QListWidget::currentRowChanged, ui->stack, &QStackedWidget::setCurrentIndex);

        connect(ui->buttonBox, &QDialogButtonBox::accepted, [=](){
           auto newMath = TraceMath::createMath(static_cast<TraceMath::Type>(ui->list->currentRow()));
           if(newMath) {
               model->addOperation(newMath);
           }
        });
        ui->list->setCurrentRow(0);
        ui->stack->setCurrentIndex(0);

        d->show();
    });
    connect(ui->bDelete, &QPushButton::clicked, [=](){
        model->deleteRow(ui->view->currentIndex().row());
    });
    connect(ui->bMoveUp, &QPushButton::clicked, [&](){
        auto index = ui->view->currentIndex();
        t.swapMathOrder(index.row() - 1);
        ui->view->setCurrentIndex(index.sibling(index.row() - 1, 0));
    });
    connect(ui->bMoveDown, &QPushButton::clicked, [&](){
        auto index = ui->view->currentIndex();
        t.swapMathOrder(index.row());
        ui->view->setCurrentIndex(index.sibling(index.row() + 1, 0));
    });
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
    Q_UNUSED(parent);
    return t.getMathOperations().size();
}

int MathModel::columnCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent);
    return ColIndexLast;
}

QVariant MathModel::data(const QModelIndex &index, int role) const
{
    if(!index.isValid()) {
        return QVariant();
    }
    auto math = t.getMathOperations().at(index.row());
    switch(index.column()) {
    case ColIndexStatus:
        if(role == Qt::DecorationRole) {
            switch(math.math->getStatus()) {
            case TraceMath::Status::Ok:
                return QApplication::style()->standardIcon(QStyle::SP_DialogApplyButton);
            case TraceMath::Status::Warning:
                return QApplication::style()->standardIcon(QStyle::SP_MessageBoxWarning);
            case TraceMath::Status::Error:
                return QApplication::style()->standardIcon(QStyle::SP_MessageBoxCritical);
            }
        } else if(role == Qt::ToolTipRole) {
            if(math.math->getStatus() != TraceMath::Status::Ok) {
                return math.math->getStatusDescription();
            }
        }
        break;
    case ColIndexDescription:
        if(role == Qt::DisplayRole) {
            return math.math->description();
        }
//        else if(role == Qt::CheckStateRole){
//            return math.enabled ? Qt::Checked : Qt::Unchecked;
//        }
        break;
    case ColIndexDomain:
        if(role == Qt::DisplayRole) {
            switch(math.math->getDataType()) {
            case TraceMath::DataType::Time:
                return "Time";
            case TraceMath::DataType::Frequency:
                return "Frequency";
            case TraceMath::DataType::Invalid:
                return "Invalid";
            }
        }
    }
    return QVariant();
}

QVariant MathModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if(orientation == Qt::Horizontal && role == Qt::DisplayRole) {
        switch(section) {
        case ColIndexStatus: return "Status"; break;
        case ColIndexDescription: return "Description"; break;
        case ColIndexDomain: return "Output domain"; break;
        default: break;
        }
    }
    return QVariant();
}

Qt::ItemFlags MathModel::flags(const QModelIndex &index) const
{
    int flags = Qt::NoItemFlags;
    if(index.row() >= 1) {
        // the first entry is always the trace itself and not enabled
        flags |= Qt::ItemIsEnabled | Qt::ItemIsSelectable;
    }
//    switch(index.column()) {
//    case ColIndexDescription: flags |= Qt::ItemIsUserCheckable; break;
//    default:
//        break;
//    }
    return (Qt::ItemFlags) flags;
}

void MathModel::addOperation(TraceMath *math)
{
    beginInsertRows(QModelIndex(), t.getMathOperations().size(), t.getMathOperations().size());
    t.addMathOperation(math);
    endInsertRows();
    // open the editor for the newly added operation
    math->edit();
}

void MathModel::deleteRow(unsigned int row)
{
    beginRemoveRows(QModelIndex(), row, row);
    t.removeMathOperation(row);
    endRemoveRows();
}

