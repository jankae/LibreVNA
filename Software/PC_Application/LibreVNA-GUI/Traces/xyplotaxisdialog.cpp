#include "xyplotaxisdialog.h"

#include "ui_xyplotaxisdialog.h"
#include "traceaxis.h"
#include "preferences.h"

#include <QFileDialog>
#include <QStandardItemModel>
#include <fstream>
#include <iomanip>

#include <CustomWidgets/informationbox.h>

using namespace std;

static void enableComboBoxItem(QComboBox *cb, int itemNum, bool enable) {
    auto *model = qobject_cast<QStandardItemModel *>(cb->model());
    auto item = model->item(itemNum);
    item->setFlags(enable ? item->flags() | Qt::ItemIsEnabled
                            : item->flags() & ~Qt::ItemIsEnabled);
}

XYplotAxisDialog::XYplotAxisDialog(TraceXYPlot *plot) :
    QDialog(nullptr),
    ui(new Ui::XYplotAxisDialog),
    plot(plot)
{
    ui->setupUi(this);
    ui->Y1type->clear();
    ui->Y2type->clear();
    ui->Y1type->setMaxVisibleItems(20);
    ui->Y2type->setMaxVisibleItems(20);

    for(int i=0;i<(int) YAxis::Type::Last;i++) {
        ui->Y1type->addItem(YAxis::TypeToName((YAxis::Type) i));
        ui->Y2type->addItem(YAxis::TypeToName((YAxis::Type) i));
    }

    for(int i=0;i<(int) XAxis::Type::Last;i++) {
        ui->XType->addItem(XAxis::TypeToName((XAxis::Type) i));
    }

    if(plot->getModel().getSource() == TraceModel::DataSource::SA) {
        for(int i=0;i<ui->XType->count();i++) {
            auto xtype = XAxis::TypeFromName(ui->XType->itemText(i));
            if(!isSupported(xtype)) {
                enableComboBoxItem(ui->XType, i, false);
            }
        }
    }

    // Setup GUI connections
    auto updateYenableState = [plot](QComboBox *type, QRadioButton *linear, QRadioButton *log, QCheckBox *CBauto, SIUnitEdit *min, SIUnitEdit *max, QSpinBox *divs, QCheckBox *autoDivs) {
        if(type->currentIndex() == 0) {
            // axis disabled
            log->setEnabled(false);
            linear->setEnabled(false);
            CBauto->setEnabled(false);
            min->setEnabled(false);
            max->setEnabled(false);
            divs->setEnabled(false);
            autoDivs->setEnabled(false);
        } else {
            // axis enabled
            log->setEnabled(true);
            linear->setEnabled(true);
            CBauto->setEnabled(true);
            if(CBauto->isChecked()) {
                // autorange active, other settings disabled
                min->setEnabled(false);
                max->setEnabled(false);
                divs->setEnabled(false);
                autoDivs->setEnabled(false);
            } else {
                min->setEnabled(true);
                max->setEnabled(true);
                if(log->isChecked()) {
                    divs->setEnabled(false);
                    autoDivs->setEnabled(false);
                } else {
                    autoDivs->setEnabled(true);
                    divs->setEnabled(!autoDivs->isChecked());
                }
            }
        }
        auto t = (YAxis::Type) type->currentIndex();
        QString unit = YAxis::Unit(t, plot->getModel().getSource());
        QString prefixes = YAxis::Prefixes(t);
        min->setUnit(unit);
        min->setPrefixes(prefixes);
        max->setUnit(unit);
        max->setPrefixes(prefixes);
    };

    auto setupYAxisConnetions = [this, updateYenableState](QComboBox *type, QRadioButton *linear, QRadioButton *log, QCheckBox *CBauto, SIUnitEdit *min, SIUnitEdit *max, QSpinBox *divs, QCheckBox *autoDivs) {
        connect(type, qOverload<int>(&QComboBox::currentIndexChanged), this, [=](int) {
            updateYenableState(type, linear, log, CBauto, min, max, divs, autoDivs);
            // update min/max settings when axis has changed
            if(type->currentIndex() != 0) {
                auto axisType = (YAxis::Type) type->currentIndex();
                min->setValue(YAxis::getDefaultLimitMin(axisType));
                max->setValue(YAxis::getDefaultLimitMax(axisType));
            }
        });
        connect(CBauto, &QCheckBox::toggled, this, [=](bool) {
            updateYenableState(type, linear, log, CBauto, min, max, divs, autoDivs);
        });
        connect(log, &QCheckBox::toggled, this, [=](bool) {
            updateYenableState(type, linear, log, CBauto, min, max, divs, autoDivs);
        });
        connect(autoDivs, &QCheckBox::toggled, this, [=](bool) {
            updateYenableState(type, linear, log, CBauto, min, max, divs, autoDivs);
        });
    };

    setupYAxisConnetions(ui->Y1type, ui->Y1linear, ui->Y1log, ui->Y1auto, ui->Y1min, ui->Y1max, ui->Y1Divs, ui->Y1autoDivs);
    setupYAxisConnetions(ui->Y2type, ui->Y2linear, ui->Y2log, ui->Y2auto, ui->Y2min, ui->Y2max, ui->Y2Divs, ui->Y2autoDivs);

    auto updateXenableState = [](QRadioButton *linear, QRadioButton *log, QCheckBox *CBauto, SIUnitEdit *min, SIUnitEdit *max, QSpinBox *divs, QCheckBox *autoDivs) {
        log->setEnabled(true);
        linear->setEnabled(true);
        CBauto->setEnabled(true);
        if(CBauto->isChecked()) {
            // autorange active, other settings disabled
            min->setEnabled(false);
            max->setEnabled(false);
            divs->setEnabled(false);
            autoDivs->setEnabled(false);
        } else {
            min->setEnabled(true);
            max->setEnabled(true);
            if(log->isChecked()) {
                divs->setEnabled(false);
                autoDivs->setEnabled(false);
            } else {
                autoDivs->setEnabled(true);
                divs->setEnabled(!autoDivs->isChecked());
            }
        }
    };

    connect(ui->Xauto, &QCheckBox::toggled, [this, updateXenableState](bool) {
        updateXenableState(ui->Xlinear, ui->Xlog, ui->Xauto, ui->Xmin, ui->Xmax, ui->XDivs, ui->XautoDivs);
    });
    connect(ui->XautoDivs, &QCheckBox::toggled, [this, updateXenableState](bool) {
        updateXenableState(ui->Xlinear, ui->Xlog, ui->Xauto, ui->Xmin, ui->Xmax, ui->XDivs, ui->XautoDivs);
    });

    ui->XType->setCurrentIndex((int) plot->xAxis.getType());
    ui->Xmin->setPrefixes("pnum kMG");
    ui->Xmax->setPrefixes("pnum kMG");

    ui->Y1min->setPrefixes("pnum kMG");
    ui->Y1max->setPrefixes("pnum kMG");

    ui->Y2min->setPrefixes("pnum kMG");
    ui->Y2max->setPrefixes("pnum kMG");

    XAxisTypeChanged((int) plot->xAxis.getType());
    connect(ui->XType, qOverload<int>(&QComboBox::currentIndexChanged), this, &XYplotAxisDialog::XAxisTypeChanged);
    connect(ui->Xlog, &QCheckBox::toggled, [=](bool){
        updateXenableState(ui->Xlinear, ui->Xlog, ui->Xauto, ui->Xmin, ui->Xmax, ui->XDivs, ui->XautoDivs);
    });

    // Fill initial values
    ui->Y1type->setCurrentIndex((int) plot->yAxis[0].getType());
    if(plot->yAxis[0].getLog()) {
        ui->Y1log->setChecked(true);
    } else {
        ui->Y1linear->setChecked(true);
    }
    ui->Y1auto->setChecked(plot->yAxis[0].getAutorange());
    ui->Y1min->setValueQuiet(plot->yAxis[0].getRangeMin());
    ui->Y1max->setValueQuiet(plot->yAxis[0].getRangeMax());
    ui->Y1Divs->setValue(plot->yAxis[0].getDivs());
    ui->Y1autoDivs->setChecked(plot->yAxis[0].getAutoDivs());

    ui->Y2type->setCurrentIndex((int) plot->yAxis[1].getType());
    if(plot->yAxis[1].getLog()) {
        ui->Y2log->setChecked(true);
    } else {
        ui->Y2linear->setChecked(true);
    }
    ui->Y2auto->setChecked(plot->yAxis[1].getAutorange());
    ui->Y2min->setValueQuiet(plot->yAxis[1].getRangeMin());
    ui->Y2max->setValueQuiet(plot->yAxis[1].getRangeMax());
    ui->Y2Divs->setValue(plot->yAxis[1].getDivs());
    ui->Y2autoDivs->setChecked(plot->yAxis[1].getAutoDivs());

    if(plot->xAxis.getLog()) {
        ui->Xlog->setChecked(true);
    } else {
        ui->Xlinear->setChecked(true);
    }
    ui->Xauto->setChecked(plot->xAxisMode != TraceXYPlot::XAxisMode::Manual);
    if(plot->xAxisMode == TraceXYPlot::XAxisMode::UseSpan) {
        ui->Xautomode->setCurrentIndex(0);
    } else {
        ui->Xautomode->setCurrentIndex(1);
    }
    ui->Xmin->setValueQuiet(plot->xAxis.getRangeMin());
    ui->Xmax->setValueQuiet(plot->xAxis.getRangeMax());
    ui->XDivs->setValue(plot->xAxis.getDivs());
    ui->XautoDivs->setChecked(plot->xAxis.getAutoDivs());

    // Constant line list handling
    auto editLine = [plot, this](XYPlotConstantLine *line) {
        line->editDialog(XAxis::Unit((XAxis::Type) ui->XType->currentIndex()),
                        YAxis::Unit((YAxis::Type) ui->Y1type->currentIndex(), plot->getModel().getSource()),
                        YAxis::Unit((YAxis::Type) ui->Y2type->currentIndex(), plot->getModel().getSource()));
    };

    for(auto l : plot->constantLines) {
        auto item = new QListWidgetItem(l->getDescription());
        ui->lineList->addItem(item);
        connect(l, &XYPlotConstantLine::editingFinished, this, [=](){
            item->setText(l->getDescription());
        });
    }
    if(plot->constantLines.size() > 0) {
        ui->removeLine->setEnabled(true);
    }
    connect(ui->addLine, &QPushButton::clicked, [=](){
        auto line = new XYPlotConstantLine();
        plot->constantLines.push_back(line);
        auto item = new QListWidgetItem(line->getDescription());
        ui->lineList->addItem(item);
        ui->lineList->setCurrentItem(item);
        ui->removeLine->setEnabled(true);
        editLine(line);
        connect(line, &XYPlotConstantLine::editingFinished, this, [=](){
            if(line->getPoints().size() < 2) {  // must have 2 points to be a line
                int index = ui->lineList->currentRow();
                removeLine(index);
            } else {
                item->setText(line->getDescription());
            }
        });
    });
    connect(ui->removeLine, &QPushButton::clicked, this, [=](){
        int index = ui->lineList->currentRow();
        removeLine(index);
    });
    connect(ui->exportLines, &QPushButton::clicked, this, [=](){
        QString filename = QFileDialog::getSaveFileName(nullptr, "Save limit lines", "", "Limit files (*.limits)", nullptr, Preferences::QFileDialogOptions());
        if(filename.isEmpty()) {
            // aborted selection
            return;
        }
        if(!filename.endsWith(".limits")) {
            filename.append(".limits");
        }

        nlohmann::json jlines;
        for(auto l : plot->constantLines ) {
            jlines.push_back(l->toJSON());
        }
        nlohmann::json j;
        j["limitLines"] = jlines;

        ofstream file;
        file.open(filename.toStdString());
        file << setw(4) << j << endl;
        file.close();

    });
    connect(ui->importLines, &QPushButton::clicked, [=](){
        QString filename = QFileDialog::getOpenFileName(nullptr, "Load limit lines", "", "Limit files (*.limits)", nullptr, Preferences::QFileDialogOptions());
        ifstream file;
        file.open(filename.toStdString());
        if(!file.is_open()) {
            qWarning() << "Unable to open file:" << filename;
            return;
        }
        nlohmann::json j;
        try {
            file >> j;
        } catch (exception &e) {
            InformationBox::ShowError("Error", "Failed to parse file (" + QString(e.what()) + ")");
            qWarning() << "Parsing of limits file failed: " << e.what();
            file.close();
            return;
        }
        file.close();
        if(j.contains("limitLines")) {
            for(auto jline : j["limitLines"]) {
                auto line = new XYPlotConstantLine;
                line->fromJSON(jline);
                plot->constantLines.push_back(line);
                auto item = new QListWidgetItem(line->getDescription());
                ui->lineList->addItem(item);

            }
        }
    });

    connect(ui->lineList, &QListWidget::itemDoubleClicked, [=](QListWidgetItem *item){
        ui->lineList->setCurrentItem(item);
        editLine(plot->constantLines[ui->lineList->currentRow()]);
    });

    connect(ui->buttonBox->button(QDialogButtonBox::Apply), &QPushButton::clicked, [=](){
       on_buttonBox_accepted();
    });
}

XYplotAxisDialog::~XYplotAxisDialog()
{
    delete ui;
}

void XYplotAxisDialog::on_buttonBox_accepted()
{
    // set plot values to the ones selected in the dialog
    plot->setYAxis(0, (YAxis::Type) ui->Y1type->currentIndex(), ui->Y1log->isChecked(), ui->Y1auto->isChecked(), ui->Y1min->value(), ui->Y1max->value(), ui->Y1Divs->value(), ui->Y1autoDivs->isChecked());
    plot->setYAxis(1, (YAxis::Type) ui->Y2type->currentIndex(), ui->Y2log->isChecked(), ui->Y2auto->isChecked(), ui->Y2min->value(), ui->Y2max->value(), ui->Y2Divs->value(), ui->Y2autoDivs->isChecked());
    TraceXYPlot::XAxisMode mode;
    if(ui->Xauto->isChecked()) {
        if(ui->Xautomode->currentIndex() == 0) {
            mode = TraceXYPlot::XAxisMode::UseSpan;
        } else {
            mode = TraceXYPlot::XAxisMode::FitTraces;
        }
    } else {
        mode = TraceXYPlot::XAxisMode::Manual;
    }
    plot->setXAxis((XAxis::Type) ui->XType->currentIndex(), mode, ui->Xlog->isChecked(), ui->Xmin->value(), ui->Xmax->value(), ui->XDivs->value(), ui->XautoDivs->isChecked());
}

void XYplotAxisDialog::XAxisTypeChanged(int XAxisIndex)
{
    auto type = (XAxis::Type) XAxisIndex;
    auto supported = supportedYAxis(type);
    for(unsigned int i=0;i<(int) YAxis::Type::Last;i++) {
        auto t = (YAxis::Type) i;
        auto enable = supported.count(t) > 0;
        auto index = (int) t;
        enableComboBoxItem(ui->Y1type, index, enable);
        enableComboBoxItem(ui->Y2type, index, enable);
    }
    // Disable Yaxis if previously selected type is not supported
    if(!supported.count((YAxis::Type)ui->Y1type->currentIndex())) {
        ui->Y1type->setCurrentIndex(0);
    }
    if(!supported.count((YAxis::Type)ui->Y2type->currentIndex())) {
        ui->Y2type->setCurrentIndex(0);
    }

    if(type == XAxis::Type::Frequency) {
        enableComboBoxItem(ui->Xautomode, 0, true);
        ui->Xlog->setEnabled(true);
    } else {
        // auto mode using span not supported in time mode
        if(ui->Xautomode->currentIndex() == 0) {
            ui->Xautomode->setCurrentIndex(1);
        }
        enableComboBoxItem(ui->Xautomode, 0, false);
        // log option only available for frequency axis
        if(ui->Xlog->isChecked()) {
            ui->Xlinear->setChecked(true);
        }
        ui->Xlog->setEnabled(false);
    }

    QString unit = XAxis::Unit(type);
    ui->Xmin->setUnit(unit);
    ui->Xmax->setUnit(unit);
}

std::set<YAxis::Type> XYplotAxisDialog::supportedYAxis(XAxis::Type type)
{
    return YAxis::getSupported(type, plot->getModel().getSource());
}

bool XYplotAxisDialog::isSupported(XAxis::Type type)
{
    return XAxis::isSupported(type, plot->getModel().getSource());
}

void XYplotAxisDialog::removeLine(int index) {
    if (index < 0) {
        return;
    }
    delete ui->lineList->takeItem(index);
    delete plot->constantLines[index];
    plot->constantLines.erase(plot->constantLines.begin() + index);
    if(plot->constantLines.size() == 0) {
        ui->removeLine->setEnabled(false);
    }
}
