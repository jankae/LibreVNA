#include "xyplotaxisdialog.h"

#include "ui_xyplotaxisdialog.h"
#include "traceaxis.h"

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
    connect(ui->Y1type, qOverload<int>(&QComboBox::currentIndexChanged), [=](int index) {
       ui->Y1log->setEnabled(index != 0);
       ui->Y1linear->setEnabled(index != 0);
       ui->Y1auto->setEnabled(index != 0);
       bool autoRange = ui->Y1auto->isChecked();
       ui->Y1min->setEnabled(index != 0 && !autoRange);
       ui->Y1max->setEnabled(index != 0 && !autoRange);
       ui->Y1divs->setEnabled(index != 0 && !autoRange);
       auto type = (YAxis::Type) index;
       QString unit = YAxis::Unit(type);
       QString prefixes = YAxis::Prefixes(type);
       ui->Y1min->setUnit(unit);
       ui->Y1min->setPrefixes(prefixes);
       ui->Y1max->setUnit(unit);
       ui->Y1max->setPrefixes(prefixes);
       ui->Y1divs->setUnit(unit);
       ui->Y1divs->setPrefixes(prefixes);
    });
    connect(ui->Y1auto, &QCheckBox::toggled, [this](bool checked) {
       ui->Y1min->setEnabled(!checked);
       ui->Y1max->setEnabled(!checked);
       ui->Y1divs->setEnabled(!checked && !ui->Y1log->isChecked());
    });
    connect(ui->Y1log, &QCheckBox::toggled, [this](bool checked) {
        ui->Y1divs->setEnabled(!checked && !ui->Y1auto->isChecked());
    });

    connect(ui->Y2type, qOverload<int>(&QComboBox::currentIndexChanged), [=](int index) {
       ui->Y2log->setEnabled(index != 0);
       ui->Y2linear->setEnabled(index != 0);
       ui->Y2auto->setEnabled(index != 0);
       bool autoRange = ui->Y2auto->isChecked();
       ui->Y2min->setEnabled(index != 0 && !autoRange);
       ui->Y2max->setEnabled(index != 0 && !autoRange);
       ui->Y2divs->setEnabled(index != 0 && !autoRange);
       auto type = (YAxis::Type) index;
       QString unit = YAxis::Unit(type);
       QString prefixes = YAxis::Prefixes(type);
       ui->Y2min->setUnit(unit);
       ui->Y2min->setPrefixes(prefixes);
       ui->Y2max->setUnit(unit);
       ui->Y2max->setPrefixes(prefixes);
       ui->Y2divs->setUnit(unit);
       ui->Y2divs->setPrefixes(prefixes);
    });

    connect(ui->Y2auto, &QCheckBox::toggled, [this](bool checked) {
       ui->Y2min->setEnabled(!checked);
       ui->Y2max->setEnabled(!checked);
       ui->Y2divs->setEnabled(!checked && !ui->Y1log->isChecked());
    });
    connect(ui->Y2log, &QCheckBox::toggled, [this](bool checked) {
        ui->Y2divs->setEnabled(!checked && !ui->Y2auto->isChecked());
    });

    connect(ui->Xauto, &QCheckBox::toggled, [this](bool checked) {
       ui->Xmin->setEnabled(!checked);
       ui->Xmax->setEnabled(!checked);
       ui->Xdivs->setEnabled(!checked && ui->Xlinear->isChecked());
       ui->Xautomode->setEnabled(checked);
    });

    ui->XType->setCurrentIndex((int) plot->xAxis.getType());
    ui->Xmin->setPrefixes("pnum kMG");
    ui->Xmax->setPrefixes("pnum kMG");
    ui->Xdivs->setPrefixes("pnum kMG");

    ui->Y1min->setPrefixes("pnum kMG");
    ui->Y1max->setPrefixes("pnum kMG");
    ui->Y1divs->setPrefixes("pnum kMG");

    ui->Y2min->setPrefixes("pnum kMG");
    ui->Y2max->setPrefixes("pnum kMG");
    ui->Y2divs->setPrefixes("pnum kMG");

    XAxisTypeChanged((int) plot->xAxis.getType());
    connect(ui->XType, qOverload<int>(&QComboBox::currentIndexChanged), this, &XYplotAxisDialog::XAxisTypeChanged);
    connect(ui->Xlog, &QCheckBox::toggled, [=](bool checked){
        ui->Xdivs->setEnabled(!checked && !ui->Xauto->isChecked());
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
    ui->Y1divs->setValueQuiet(plot->yAxis[0].getRangeDiv());

    ui->Y2type->setCurrentIndex((int) plot->yAxis[1].getType());
    if(plot->yAxis[1].getLog()) {
        ui->Y2log->setChecked(true);
    } else {
        ui->Y2linear->setChecked(true);
    }
    ui->Y2auto->setChecked(plot->yAxis[1].getAutorange());
    ui->Y2min->setValueQuiet(plot->yAxis[1].getRangeMin());
    ui->Y2max->setValueQuiet(plot->yAxis[1].getRangeMax());
    ui->Y2divs->setValueQuiet(plot->yAxis[1].getRangeDiv());

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
    ui->Xdivs->setValueQuiet(plot->xAxis.getRangeDiv());

    // Constant line list handling
    auto editLine = [&](XYPlotConstantLine *line) {
        line->editDialog(XAxis::Unit((XAxis::Type) ui->XType->currentIndex()),
                        YAxis::Unit((YAxis::Type) ui->Y1type->currentIndex()),
                        YAxis::Unit((YAxis::Type) ui->Y2type->currentIndex()));
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
        QString filename = QFileDialog::getSaveFileName(nullptr, "Save limit lines", "", "Limit files (*.limits)", nullptr, QFileDialog::DontUseNativeDialog);
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
        QString filename = QFileDialog::getOpenFileName(nullptr, "Load limit lines", "", "Limit files (*.limits)", nullptr, QFileDialog::DontUseNativeDialog);
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
    plot->setYAxis(0, (YAxis::Type) ui->Y1type->currentIndex(), ui->Y1log->isChecked(), ui->Y1auto->isChecked(), ui->Y1min->value(), ui->Y1max->value(), ui->Y1divs->value());
    plot->setYAxis(1, (YAxis::Type) ui->Y2type->currentIndex(), ui->Y2log->isChecked(), ui->Y2auto->isChecked(), ui->Y2min->value(), ui->Y2max->value(), ui->Y2divs->value());
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
    plot->setXAxis((XAxis::Type) ui->XType->currentIndex(), mode, ui->Xlog->isChecked(), ui->Xmin->value(), ui->Xmax->value(), ui->Xdivs->value());
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
    ui->Xdivs->setUnit(unit);
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
