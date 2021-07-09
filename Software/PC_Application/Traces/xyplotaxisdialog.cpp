#include "xyplotaxisdialog.h"
#include "ui_xyplotaxisdialog.h"
#include <QStandardItemModel>

using namespace std;

XYplotAxisDialog::XYplotAxisDialog(TraceXYPlot *plot) :
    QDialog(nullptr),
    ui(new Ui::XYplotAxisDialog),
    plot(plot)
{
    ui->setupUi(this);
    ui->Y1type->clear();
    ui->Y2type->clear();

    for(int i=0;i<(int) TraceXYPlot::YAxisType::Last;i++) {
        ui->Y1type->addItem(TraceXYPlot::AxisTypeToName((TraceXYPlot::YAxisType) i));
        ui->Y2type->addItem(TraceXYPlot::AxisTypeToName((TraceXYPlot::YAxisType) i));
    }

    for(int i=0;i<(int) TraceXYPlot::XAxisType::Last;i++) {
        ui->XType->addItem(TraceXYPlot::AxisTypeToName((TraceXYPlot::XAxisType) i));
    }

    // Setup GUI connections
    connect(ui->Y1type, qOverload<int>(&QComboBox::currentIndexChanged), [this](int index) {
       //ui->Y1log->setEnabled(index != 0);
       ui->Y1linear->setEnabled(index != 0);
       ui->Y1auto->setEnabled(index != 0);
       bool autoRange = ui->Y1auto->isChecked();
       ui->Y1min->setEnabled(index != 0 && !autoRange);
       ui->Y1max->setEnabled(index != 0 && !autoRange);
       ui->Y1divs->setEnabled(index != 0 && !autoRange);
       auto type = (TraceXYPlot::YAxisType) index;
       QString unit = TraceXYPlot::AxisUnit(type);
       ui->Y1min->setUnit(unit);
       ui->Y1max->setUnit(unit);
       ui->Y1divs->setUnit(unit);
    });
    connect(ui->Y1auto, &QCheckBox::toggled, [this](bool checked) {
       ui->Y1min->setEnabled(!checked);
       ui->Y1max->setEnabled(!checked);
       ui->Y1divs->setEnabled(!checked);
    });

    connect(ui->Y2type, qOverload<int>(&QComboBox::currentIndexChanged), [this](int index) {
       //ui->Y2log->setEnabled(index != 0);
       ui->Y2linear->setEnabled(index != 0);
       ui->Y2auto->setEnabled(index != 0);
       bool autoRange = ui->Y2auto->isChecked();
       ui->Y2min->setEnabled(index != 0 && !autoRange);
       ui->Y2max->setEnabled(index != 0 && !autoRange);
       ui->Y2divs->setEnabled(index != 0 && !autoRange);
       auto type = (TraceXYPlot::YAxisType) index;
       QString unit = TraceXYPlot::AxisUnit(type);
       ui->Y2min->setUnit(unit);
       ui->Y2max->setUnit(unit);
       ui->Y2divs->setUnit(unit);
    });

    connect(ui->Y2auto, &QCheckBox::toggled, [this](bool checked) {
       ui->Y2min->setEnabled(!checked);
       ui->Y2max->setEnabled(!checked);
       ui->Y2divs->setEnabled(!checked);
    });

    connect(ui->Xauto, &QCheckBox::toggled, [this](bool checked) {
       ui->Xmin->setEnabled(!checked);
       ui->Xmax->setEnabled(!checked);
       ui->Xdivs->setEnabled(!checked);
       ui->Xautomode->setEnabled(checked);
    });

    ui->XType->setCurrentIndex((int) plot->XAxis.type);
    ui->Xmin->setPrefixes("pnum kMG");
    ui->Xmax->setPrefixes("pnum kMG");
    ui->Xdivs->setPrefixes("pnum kMG");

    XAxisTypeChanged((int) plot->XAxis.type);
    connect(ui->XType, qOverload<int>(&QComboBox::currentIndexChanged), this, &XYplotAxisDialog::XAxisTypeChanged);

    // Fill initial values
    ui->Y1type->setCurrentIndex((int) plot->YAxis[0].type);
    if(plot->YAxis[0].log) {
        ui->Y1log->setChecked(true);
    } else {
        ui->Y1linear->setChecked(true);
    }
    ui->Y1auto->setChecked(plot->YAxis[0].autorange);
    ui->Y1min->setValueQuiet(plot->YAxis[0].rangeMin);
    ui->Y1max->setValueQuiet(plot->YAxis[0].rangeMax);
    ui->Y1divs->setValueQuiet(plot->YAxis[0].rangeDiv);

    ui->Y2type->setCurrentIndex((int) plot->YAxis[1].type);
    if(plot->YAxis[1].log) {
        ui->Y2log->setChecked(true);
    } else {
        ui->Y2linear->setChecked(true);
    }
    ui->Y2auto->setChecked(plot->YAxis[1].autorange);
    ui->Y2min->setValueQuiet(plot->YAxis[1].rangeMin);
    ui->Y2max->setValueQuiet(plot->YAxis[1].rangeMax);
    ui->Y2divs->setValueQuiet(plot->YAxis[1].rangeDiv);

    ui->Xauto->setChecked(plot->XAxis.mode != TraceXYPlot::XAxisMode::Manual);
    if(plot->XAxis.mode == TraceXYPlot::XAxisMode::UseSpan) {
        ui->Xautomode->setCurrentIndex(0);
    } else {
        ui->Xautomode->setCurrentIndex(1);
    }
    ui->Xmin->setValueQuiet(plot->XAxis.rangeMin);
    ui->Xmax->setValueQuiet(plot->XAxis.rangeMax);
    ui->Xdivs->setValueQuiet(plot->XAxis.rangeDiv);
}

XYplotAxisDialog::~XYplotAxisDialog()
{
    delete ui;
}

void XYplotAxisDialog::on_buttonBox_accepted()
{
    // set plot values to the ones selected in the dialog
    plot->setYAxis(0, (TraceXYPlot::YAxisType) ui->Y1type->currentIndex(), ui->Y1log->isChecked(), ui->Y1auto->isChecked(), ui->Y1min->value(), ui->Y1max->value(), ui->Y1divs->value());
    plot->setYAxis(1, (TraceXYPlot::YAxisType) ui->Y2type->currentIndex(), ui->Y2log->isChecked(), ui->Y2auto->isChecked(), ui->Y2min->value(), ui->Y2max->value(), ui->Y2divs->value());
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
    plot->setXAxis((TraceXYPlot::XAxisType) ui->XType->currentIndex(), mode, ui->Xmin->value(), ui->Xmax->value(), ui->Xdivs->value());
}

static void enableComboBoxItem(QComboBox *cb, int itemNum, bool enable) {
    auto *model = qobject_cast<QStandardItemModel *>(cb->model());
    auto item = model->item(itemNum);
    item->setFlags(enable ? item->flags() | Qt::ItemIsEnabled
                            : item->flags() & ~Qt::ItemIsEnabled);
}

void XYplotAxisDialog::XAxisTypeChanged(int XAxisIndex)
{
    auto type = (TraceXYPlot::XAxisType) XAxisIndex;
    auto supported = supportedYAxis(type);
    for(auto t : TraceXYPlot::YAxisTypes) {
        auto enable = supported.count(t) > 0;
        auto index = (int) t;
        enableComboBoxItem(ui->Y1type, index, enable);
        enableComboBoxItem(ui->Y2type, index, enable);
    }
    // Disable Yaxis if previously selected type is not supported
    if(!supported.count((TraceXYPlot::YAxisType)ui->Y1type->currentIndex())) {
        ui->Y1type->setCurrentIndex(0);
    }
    if(!supported.count((TraceXYPlot::YAxisType)ui->Y2type->currentIndex())) {
        ui->Y2type->setCurrentIndex(0);
    }

    if(type == TraceXYPlot::XAxisType::Frequency) {
        enableComboBoxItem(ui->Xautomode, 0, true);
    } else {
        // auto mode using span not supported in time mode
        if(ui->Xautomode->currentIndex() == 0) {
            ui->Xautomode->setCurrentIndex(1);
            enableComboBoxItem(ui->Xautomode, 0, false);
        }
    }

    QString unit = TraceXYPlot::AxisUnit(type);
    ui->Xmin->setUnit(unit);
    ui->Xmax->setUnit(unit);
    ui->Xdivs->setUnit(unit);
}

std::set<TraceXYPlot::YAxisType> XYplotAxisDialog::supportedYAxis(TraceXYPlot::XAxisType type)
{
    set<TraceXYPlot::YAxisType> ret = {TraceXYPlot::YAxisType::Disabled};
    switch(type) {
    case TraceXYPlot::XAxisType::Frequency:
    case TraceXYPlot::XAxisType::Power:
        ret.insert(TraceXYPlot::YAxisType::Magnitude);
        ret.insert(TraceXYPlot::YAxisType::Phase);
        ret.insert(TraceXYPlot::YAxisType::VSWR);
        ret.insert(TraceXYPlot::YAxisType::SeriesR);
        ret.insert(TraceXYPlot::YAxisType::Capacitance);
        ret.insert(TraceXYPlot::YAxisType::Inductance);
        ret.insert(TraceXYPlot::YAxisType::QualityFactor);
        break;
    case TraceXYPlot::XAxisType::Time:
    case TraceXYPlot::XAxisType::Distance:
        ret.insert(TraceXYPlot::YAxisType::ImpulseReal);
        ret.insert(TraceXYPlot::YAxisType::ImpulseMag);
        ret.insert(TraceXYPlot::YAxisType::Step);
        ret.insert(TraceXYPlot::YAxisType::Impedance);
        break;
    default:
        break;
    }
    return ret;
}
