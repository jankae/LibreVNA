#include "traceeditdialog.h"

#include "ui_traceeditdialog.h"
#include "ui_newtracemathdialog.h"
#include "Math/tdr.h"
#include "appwindow.h"
#include "CustomWidgets/informationbox.h"

#include <QColorDialog>
#include <QFileDialog>

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
    ui->impedance->setUnit("Ω");
    ui->impedance->setPrecision(3);
    ui->impedance->setValue(t.getReferenceImpedance());

    if(!t.getModel()) {
        // without information about the other traces in the model, math is not available as a source
        ui->bMath->setEnabled(false);
    }

    connect(ui->bLive, &QPushButton::clicked, [=](bool live) {
        if(live) {
            ui->stack->setCurrentIndex(0);
            ui->buttonBox->button(QDialogButtonBox::Ok)->setEnabled(true);
            ui->impedance->setEnabled(false);
        }
    });
    connect(ui->bFile, &QPushButton::clicked, [&](bool file) {
        if(file) {
            if(t.getFilename().endsWith(".csv")) {
                ui->stack->setCurrentIndex(2);
                ui->csvImport->setFile(t.getFilename());
                ui->csvImport->selectTrace(t.getFileParameter());
            } else {
                // attempt to parse as touchstone
                ui->stack->setCurrentIndex(1);
                ui->touchstoneImport->setFile(t.getFilename());
                ui->impedance->setEnabled(false);
            }
        }
    });
    connect(ui->bMath, &QPushButton::clicked, [&](bool math){
       if(math) {
           ui->stack->setCurrentIndex(3);
           ui->buttonBox->button(QDialogButtonBox::Ok)->setEnabled(updateMathFormulaStatus());
           ui->impedance->setEnabled(true);
       }
    });

    connect(ui->color, &ColorPickerButton::colorChanged, [=](const QColor& color){
       trace.setColor(color);
    });

    ui->GSource->setId(ui->bLive, 0);
    ui->GSource->setId(ui->bFile, 1);
    ui->GSource->setId(ui->bMath, 2);

    if(t.getSource() == Trace::Source::Calibration) {
        // prevent editing imported calibration traces (and csv files for now)
        ui->bLive->setEnabled(false);
        ui->bFile->setEnabled(false);
        ui->bMath->setEnabled(false);
        ui->CLiveType->setEnabled(false);
        ui->CLiveParam->setEnabled(false);
    }

    auto updateTouchstoneFileStatus = [this]() {
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
        if(ui->touchstoneImport->getFilename().endsWith(".csv")) {
            // switch to csv import dialog
            ui->stack->setCurrentIndex(2);
            ui->csvImport->setFile(ui->touchstoneImport->getFilename());
        }
    };

    auto updateCSVFileStatus = [this]() {
        if (ui->bFile->isChecked() && !ui->csvImport->getStatus())  {
            ui->buttonBox->button(QDialogButtonBox::Ok)->setEnabled(false);
        } else {
            ui->buttonBox->button(QDialogButtonBox::Ok)->setEnabled(true);
        }
        if(!(ui->touchstoneImport->getFilename().endsWith(".csv"))) {
            // switch to touchstone import dialog
            ui->stack->setCurrentIndex(1);
            ui->touchstoneImport->setFile(ui->csvImport->getFilename());
        }
    };

    switch(t.liveType()) {
    case Trace::LivedataType::Overwrite: ui->CLiveType->setCurrentIndex(0); break;
    case Trace::LivedataType::MaxHold: ui->CLiveType->setCurrentIndex(1); break;
    case Trace::LivedataType::MinHold: ui->CLiveType->setCurrentIndex(2); break;
    default: break;
    }

    VNAtrace = Trace::isVNAParameter(t.liveParameter());
    if(DeviceDriver::getActiveDriver()) {
        if(VNAtrace) {
            ui->CLiveParam->addItems(DeviceDriver::getActiveDriver()->availableVNAMeasurements());
        } else {
            ui->CLiveParam->addItems(DeviceDriver::getActiveDriver()->availableSAMeasurements());
        }
    }

    if(ui->CLiveParam->findText(t.liveParameter()) < 0) {
        ui->CLiveParam->addItem(t.liveParameter());
    }
    ui->CLiveParam->setCurrentText(t.liveParameter());

    connect(ui->touchstoneImport, &TouchstoneImport::statusChanged, updateTouchstoneFileStatus);
    connect(ui->touchstoneImport, &TouchstoneImport::filenameChanged, updateTouchstoneFileStatus);
    connect(ui->csvImport, &CSVImport::filenameChanged, updateCSVFileStatus);

    // Math source configuration
    ui->lMathFormula->setText(t.getMathFormula());
    if(t.getModel()) {
        connect(ui->lMathFormula, &QLineEdit::textChanged, [&](){
            t.setMathFormula(ui->lMathFormula->text());
            ui->buttonBox->button(QDialogButtonBox::Ok)->setEnabled(updateMathFormulaStatus());
        });

        ui->mathTraceTable->setColumnCount(2);
        ui->mathTraceTable->setHorizontalHeaderItem(0, new QTableWidgetItem("Trace Name"));
        ui->mathTraceTable->setHorizontalHeaderItem(1, new QTableWidgetItem("Variable Name"));
        auto traces = t.getModel()->getTraces();
        ui->mathTraceTable->setRowCount(traces.size());
        for(unsigned int i=0;i<traces.size();i++) {
            auto ts = traces[i];
            auto traceItem = new QTableWidgetItem(ts->name());
            auto flags = traceItem->flags() | Qt::ItemIsUserCheckable;
            flags &= ~(Qt::ItemIsEditable | Qt::ItemIsEnabled);
            if(t.canAddAsMathSource(ts)) {
                flags |= Qt::ItemIsEnabled;
            }
            traceItem->setFlags(flags);
            auto variableItem = new QTableWidgetItem(t.getSourceVariableName(ts));
            variableItem->setFlags(variableItem->flags() & ~Qt::ItemIsEditable);
            if(t.mathDependsOn(ts, true)) {
                traceItem->setCheckState(Qt::Checked);
                variableItem->setFlags(variableItem->flags() | Qt::ItemIsEnabled | Qt::ItemIsEditable);
                ui->mathTraceTable->blockSignals(false);
            } else {
                traceItem->setCheckState(Qt::Unchecked);
            }
            ui->mathTraceTable->setItem(i, 0, traceItem);
            ui->mathTraceTable->setItem(i, 1, variableItem);
        }
        connect(ui->mathTraceTable, &QTableWidget::itemChanged, [&](QTableWidgetItem *item){
            auto row = ui->mathTraceTable->row(item);
            auto column = ui->mathTraceTable->column(item);
//            qDebug() << "Item changed at row"<<row<<"column"<<column;
            ui->mathTraceTable->blockSignals(true);
            auto trace = t.getModel()->trace(row);
            if(column == 0) {
                auto variableItem = ui->mathTraceTable->item(row, 1);
                // checked state changed
                if(item->checkState() == Qt::Checked) {
                    // add this trace to the math sources, enable editing of variable name
                    t.addMathSource(trace, trace->name());
                    variableItem->setText(trace->name());
                    variableItem->setFlags(variableItem->flags() | Qt::ItemIsEnabled | Qt::ItemIsEditable);
                } else {
                    // trace disabled, remove from math sources
                    t.removeMathSource(trace);
                    variableItem->setText("");
                    variableItem->setFlags(variableItem->flags() & ~(Qt::ItemIsEnabled | Qt::ItemIsEditable));
                }
                updateMathFormulaSelectableRows();
            } else {
                // changed the variable name text
                t.addMathSource(trace, item->text());
            }
            ui->mathTraceTable->blockSignals(false);
            ui->buttonBox->button(QDialogButtonBox::Ok)->setEnabled(updateMathFormulaStatus());
        });
    }
    updateMathFormulaSelectableRows();
    updateMathFormulaStatus();

    switch(t.getSource()) {
    case Trace::Source::Live: ui->bLive->click(); break;
    case Trace::Source::File: ui->bFile->click(); break;
    case Trace::Source::Math: ui->bMath->click(); break;
    default: break;
    }

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
        connect(d, &QDialog::rejected, [=](){
            delete ui;
        });
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
        connect(d, &QDialog::accepted, [=](){
            auto type = static_cast<TraceMath::Type>(ui->list->currentRow());
            delete ui;
            auto newMath = TraceMath::createMath(type);
            model->addOperations(newMath);
            if(newMath.size() == 1) {
                // any normal math operation added, edit now
                newMath[0]->edit();
            } else {
                // composite operation added, check which one and edit the correct suboperation
                switch(type) {
                case TraceMath::Type::TimeDomainGating: {
                    auto inputData = newMath[0]->getInput()->getData();
                    // Automatically select bandpass/lowpass TDR, depending on selected span
                    if(inputData.size() > 0) {
                        // Automatically select bandpass/lowpass TDR, depending on selected span
                        auto tdr = (Math::TDR*) newMath[0];
                        auto fstart = inputData.front().x;
                        auto fstop = inputData.back().x;

                        if(fstart < fstop / 100.0) {
                            tdr->setMode(Math::TDR::Mode::Lowpass);
                        } else {
                            // lowpass mode would result in very few points in the time domain, switch to bandpass mode
                            tdr->setMode(Math::TDR::Mode::Bandpass);
                        }
                    }

                    // TDR/DFT can be left at default, edit the actual gate
                    newMath[1]->edit();
                }
                   break;
                default:
                   break;
                }
            }
        });
        ui->list->setCurrentRow(0);
        ui->stack->setCurrentIndex(0);

        if(AppWindow::showGUI()) {
            d->show();
        }
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
    connect(ui->buttonBox->button(QDialogButtonBox::Ok), &QPushButton::clicked, this, &TraceEditDialog::okClicked);
}

TraceEditDialog::~TraceEditDialog()
{
    delete ui;
}

void TraceEditDialog::okClicked()
{
    trace.setName(ui->name->text());
    trace.setVelocityFactor(ui->vFactor->value());
    trace.setReferenceImpedance(ui->impedance->value());
    if(trace.getSource() != Trace::Source::Calibration) {
        // only apply changes if it is not a calibration trace
        if (ui->bFile->isChecked()) {
            auto newName = ui->stack->currentIndex() == 1 ? ui->touchstoneImport->getFilename() : ui->csvImport->getFilename();
            if(newName != trace.getFilename()) {
                // only update if filename has changed
                if(trace.deembeddingAvailable()) {
                    InformationBox::ShowMessage("Removing de-embedding data", "You have selected a new file as the trace source. Any de-embedding data has been deleted before loading the new trace data.");
                    trace.clearDeembedding();
                }
                if(ui->stack->currentIndex() == 1) {
                    // touchstone page active
                    if(ui->touchstoneImport->getFilename() != trace.getFilename()) {
                        auto t = ui->touchstoneImport->getTouchstone();
                        trace.fillFromTouchstone(t, ui->CParameter->currentIndex());
                    }
                } else {
                    // CSV page active
                    if(ui->csvImport->getFilename() != trace.getFilename()) {

                    }
                    ui->csvImport->fillTrace(trace);
                }
            }
        } else if(ui->bLive->isChecked()) {
            Trace::LivedataType type = Trace::LivedataType::Overwrite;
            switch(ui->CLiveType->currentIndex()) {
            case 0: type = Trace::LivedataType::Overwrite; break;
            case 1: type = Trace::LivedataType::MaxHold; break;
            case 2: type = Trace::LivedataType::MinHold; break;
            }
            trace.fromLivedata(type, ui->CLiveParam->currentText());
        } else {
            // math operation trace
            trace.fromMath();
        }
    }
    delete this;
}

bool TraceEditDialog::updateMathFormulaStatus()
{
    // check output domains first (there could be a problem if a trace changed its output domain
    // after the math trace was created)
    auto domain = TraceMath::DataType::Invalid;
    for(auto t : trace.mathSourceTraces) {
        if(domain == TraceMath::DataType::Invalid) {
            domain = t.first->outputType();
        } else {
            if(domain != t.first->outputType()) {
                // not all traces have the same domain
                ui->lMathFormulaStatus->setText("Different output domains of selected source traces");
                ui->lMathFormulaStatus->setStyleSheet("QLabel { color : red; }");
                return false;
            }
        }
    }
    auto error = trace.getMathFormulaError();
    if(error.isEmpty()) {
        // all good
        ui->lMathFormulaStatus->setText("Math formula valid");
        ui->lMathFormulaStatus->setStyleSheet("");
        return true;
    } else {
        ui->lMathFormulaStatus->setText(error);
        ui->lMathFormulaStatus->setStyleSheet("QLabel { color : red; }");
        return false;
    }
}

void TraceEditDialog::updateMathFormulaSelectableRows()
{
    // available trace selections may have changed, disable/enable other rows

    // block signals, otherwise the trace names will be reset
    ui->mathTraceTable->blockSignals(true);
    for(unsigned int i=0;i<trace.getModel()->getTraces().size();i++) {
        auto traceItem = ui->mathTraceTable->item(i, 0);
        auto flags = traceItem->flags();
        if(trace.canAddAsMathSource(trace.getModel()->trace(i)) || traceItem->checkState()) {
            // Item can always be deselected but only selected if it is compatible
            traceItem->setFlags(flags | Qt::ItemIsEnabled);
        } else {
            traceItem->setFlags(flags & ~Qt::ItemIsEnabled);
        }
    }
    ui->mathTraceTable->blockSignals(false);
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
            case TraceMath::DataType::Power:
                return "Power";
            case TraceMath::DataType::TimeZeroSpan:
                return "Time (Zero Span)";
            case TraceMath::DataType::Invalid:
            default:
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

