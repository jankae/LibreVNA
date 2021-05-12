#include "traceeditdialog.h"
#include "ui_traceeditdialog.h"
#include <QColorDialog>
#include <QFileDialog>
#include "ui_newtracemathdialog.h"
#include "Math/tdr.h"

namespace Ui {
class NewTraceMathDialog;
}

TraceEditDialog::TraceEditDialog(Trace &t, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::TraceEditDialog),
    trace(t)
{
    ui->setupUi(this);
    ui->vFactor->setPrecision(3);
    ui->name->setText(t.name());
    ui->color->setColor(trace.color());
    ui->vFactor->setValue(t.velocityFactor());
    connect(ui->color, &ColorPickerButton::colorChanged, [=](const QColor& color){
       trace.setColor(color);
    });

    ui->GSource->setId(ui->bLive, 0);
    ui->GSource->setId(ui->bFile, 1);

    if(t.isCalibration() || (t.isFromFile() && t.getFilename().endsWith(".csv"))) {
        // prevent editing imported calibration traces (and csv files for now)
        ui->bLive->setEnabled(false);
        ui->bFile->setEnabled(false);
        ui->CLiveType->setEnabled(false);
        ui->CLiveParam->setEnabled(false);
    }

    if(t.isFromFile() && !t.getFilename().endsWith(".csv")) {
        ui->bFile->click();
        ui->touchstoneImport->setFile(t.getFilename());
    }

    auto updateFileStatus = [this]() {
        // remove all options from paramater combo box
        while(ui->CParameter->count() > 0) {
            ui->CParameter->removeItem(0);
        }
        if (ui->bFile->isChecked() && !ui->touchstoneImport->getStatus())  {
            ui->buttonBox->button(QDialogButtonBox::Ok)->setEnabled(false);
        } else {
            ui->buttonBox->button(QDialogButtonBox::Ok)->setEnabled(true);
            auto touchstone = ui->touchstoneImport->getTouchstone();
            for(unsigned int i=0;i<touchstone.ports();i++) {
                for(unsigned int j=0;j<touchstone.ports();j++) {
                    QString name = "S"+QString::number(i+1)+QString::number(j+1);
                    ui->CParameter->addItem(name);
                }
            }
            if(trace.getFileParameter() < touchstone.ports()*touchstone.ports()) {
                ui->CParameter->setCurrentIndex(trace.getFileParameter());
            } else {
                ui->CParameter->setCurrentIndex(0);
            }
        }
    };

    switch(t.liveType()) {
    case Trace::LivedataType::Overwrite: ui->CLiveType->setCurrentIndex(0); break;
    case Trace::LivedataType::MaxHold: ui->CLiveType->setCurrentIndex(1); break;
    case Trace::LivedataType::MinHold: ui->CLiveType->setCurrentIndex(2); break;
    default: break;
    }

    VNAtrace = Trace::isVNAParameter(t.liveParameter());
    if(VNAtrace) {
        ui->CLiveParam->addItem("S11");
        ui->CLiveParam->addItem("S12");
        ui->CLiveParam->addItem("S21");
        ui->CLiveParam->addItem("S22");
    } else {
        ui->CLiveParam->addItem("Port 1");
        ui->CLiveParam->addItem("Port 2");
    }

    switch(t.liveParameter()) {
    case Trace::LiveParameter::S11: ui->CLiveParam->setCurrentIndex(0); break;
    case Trace::LiveParameter::S12: ui->CLiveParam->setCurrentIndex(1); break;
    case Trace::LiveParameter::S21: ui->CLiveParam->setCurrentIndex(2); break;
    case Trace::LiveParameter::S22: ui->CLiveParam->setCurrentIndex(3); break;
    case Trace::LiveParameter::Port1: ui->CLiveParam->setCurrentIndex(0); break;
    case Trace::LiveParameter::Port2: ui->CLiveParam->setCurrentIndex(1); break;
    default: break;
    }

    connect(ui->GSource, qOverload<int>(&QButtonGroup::buttonClicked), updateFileStatus);
    connect(ui->touchstoneImport, &TouchstoneImport::statusChanged, updateFileStatus);
    connect(ui->touchstoneImport, &TouchstoneImport::filenameChanged, updateFileStatus);

    updateFileStatus();

    // setup math part of the GUI
    auto model = new MathModel(t);
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

        connect(ui->list, &QListWidget::doubleClicked, ui->buttonBox, &QDialogButtonBox::accepted);
        connect(ui->buttonBox, &QDialogButtonBox::accepted, [=](){
            auto type = static_cast<TraceMath::Type>(ui->list->currentRow());
            auto newMath = TraceMath::createMath(type);
            model->addOperations(newMath);
            if(newMath.size() == 1) {
                // any normal math operation added, edit now
                newMath[0]->edit();
            } else {
                // composite operation added, check which one and edit the correct suboperation
                switch(type) {
                case TraceMath::Type::TimeDomainGating:
                    // Automatically select bandpass/lowpass TDR, depending on selected span
                    if(newMath[0]->getInput()->rData().size() > 0) {
                        // Automatically select bandpass/lowpass TDR, depending on selected span
                        auto tdr = (Math::TDR*) newMath[0];
                        auto fstart = tdr->getInput()->rData().front().x;
                        auto fstop = tdr->getInput()->rData().back().x;

                        if(fstart < fstop / 100.0) {
                            tdr->setMode(Math::TDR::Mode::Lowpass);
                        } else {
                            // lowpass mode would result in very few points in the time domain, switch to bandpass mode
                            tdr->setMode(Math::TDR::Mode::Bandpass);
                        }
                    }

                    // TDR/DFT can be left at default, edit the actual gate
                    newMath[1]->edit();
                   break;
                default:
                   break;
                }
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

TraceEditDialog::~TraceEditDialog()
{
    delete ui;
}

void TraceEditDialog::on_buttonBox_accepted()
{
    trace.setName(ui->name->text());
    trace.setVelocityFactor(ui->vFactor->value());
    if(!trace.isCalibration()) {
        // only apply changes if it is not a calibration trace
        if (ui->bFile->isChecked()) {
            auto t = ui->touchstoneImport->getTouchstone();
            trace.fillFromTouchstone(t, ui->CParameter->currentIndex());
        } else {
            Trace::LivedataType type = Trace::LivedataType::Overwrite;
            Trace::LiveParameter param = Trace::LiveParameter::S11;
            switch(ui->CLiveType->currentIndex()) {
            case 0: type = Trace::LivedataType::Overwrite; break;
            case 1: type = Trace::LivedataType::MaxHold; break;
            case 2: type = Trace::LivedataType::MinHold; break;
            }
            if(VNAtrace) {
                switch(ui->CLiveParam->currentIndex()) {
                case 0: param = Trace::LiveParameter::S11; break;
                case 1: param = Trace::LiveParameter::S12; break;
                case 2: param = Trace::LiveParameter::S21; break;
                case 3: param = Trace::LiveParameter::S22; break;
                }
            } else {
                switch(ui->CLiveParam->currentIndex()) {
                case 0: param = Trace::LiveParameter::Port1; break;
                case 1: param = Trace::LiveParameter::Port2; break;
                }
            }
            trace.fromLivedata(type, param);
        }
    }
    delete this;
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
}

void MathModel::addOperations(std::vector<TraceMath *> maths)
{
    beginInsertRows(QModelIndex(), t.getMathOperations().size(), t.getMathOperations().size() + maths.size() - 1);
    t.addMathOperations(maths);
    endInsertRows();
}

void MathModel::deleteRow(unsigned int row)
{
    beginRemoveRows(QModelIndex(), row, row);
    t.removeMathOperation(row);
    endRemoveRows();
}

