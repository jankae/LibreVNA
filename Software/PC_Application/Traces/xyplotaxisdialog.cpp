#include "xyplotaxisdialog.h"
#include "ui_bodeplotaxisdialog.h"
#include <QStandardItemModel>

using namespace std;

XYplotAxisDialog::XYplotAxisDialog(TraceXYPlot *plot) :
    QDialog(nullptr),
    ui(new Ui::XYplotAxisDialog),
    plot(plot)
{
    ui->setupUi(this);

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
       QString unit = YAxisUnit(type);
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
       QString unit = YAxisUnit(type);
       ui->Y2min->setUnit(unit);
       ui->Y2max->setUnit(unit);
       ui->Y2divs->setUnit(unit);
    });

    connect(ui->XType, qOverload<int>(&QComboBox::currentIndexChanged), this, &XYplotAxisDialog::XAxisTypeChanged);

    connect(ui->Y2auto, &QCheckBox::toggled, [this](bool checked) {
       ui->Y2min->setEnabled(!checked);
       ui->Y2max->setEnabled(!checked);
       ui->Y2divs->setEnabled(!checked);
    });

    connect(ui->Xauto, &QCheckBox::toggled, [this](bool checked) {
       ui->Xmin->setEnabled(!checked);
       ui->Xmax->setEnabled(!checked);
       ui->Xdivs->setEnabled(!checked);
    });

    ui->XType->setCurrentIndex((int) plot->XAxis.Xtype);
    ui->Xmin->setPrefixes("pnum kMG");
    ui->Xmax->setPrefixes("pnum kMG");
    ui->Xdivs->setPrefixes("pnum kMG");

    // Fill initial values
    // assume same order in YAxisType enum as in ComboBox items
    ui->Y1type->setCurrentIndex((int) plot->YAxis[0].Ytype);
    if(plot->YAxis[0].log) {
        ui->Y1log->setChecked(true);
    } else {
        ui->Y1linear->setChecked(true);
    }
    ui->Y1auto->setChecked(plot->YAxis[0].autorange);
    ui->Y1min->setValueQuiet(plot->YAxis[0].rangeMin);
    ui->Y1max->setValueQuiet(plot->YAxis[0].rangeMax);
    ui->Y1divs->setValueQuiet(plot->YAxis[0].rangeDiv);

    ui->Y2type->setCurrentIndex((int) plot->YAxis[1].Ytype);
    if(plot->YAxis[1].log) {
        ui->Y2log->setChecked(true);
    } else {
        ui->Y2linear->setChecked(true);
    }
    ui->Y2auto->setChecked(plot->YAxis[1].autorange);
    ui->Y2min->setValueQuiet(plot->YAxis[1].rangeMin);
    ui->Y2max->setValueQuiet(plot->YAxis[1].rangeMax);
    ui->Y2divs->setValueQuiet(plot->YAxis[1].rangeDiv);

    ui->Xauto->setChecked(plot->XAxis.autorange);
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
    plot->setXAxis((TraceXYPlot::XAxisType) ui->XType->currentIndex(), ui->Xauto->isChecked(), ui->Xmin->value(), ui->Xmax->value(), ui->Xdivs->value());
}

void XYplotAxisDialog::XAxisTypeChanged(int XAxisIndex)
{
    auto type = (TraceXYPlot::XAxisType) XAxisIndex;
    auto supported = supportedYAxis(type);
    for(auto t : TraceXYPlot::YAxisTypes) {
        auto enable = supported.count(t) > 0;
        auto index = (int) t;
        auto *model = qobject_cast<QStandardItemModel *>(ui->Y1type->model());
        auto item = model->item(index);
        item->setFlags(enable ? item->flags() | Qt::ItemIsEnabled
                                : item->flags() & ~Qt::ItemIsEnabled);
        model = qobject_cast<QStandardItemModel *>(ui->Y2type->model());
        item = model->item(index);
        item->setFlags(enable ? item->flags() | Qt::ItemIsEnabled
                                : item->flags() & ~Qt::ItemIsEnabled);
    }
    // Disable Yaxis if previously selected type is not supported
    if(!supported.count((TraceXYPlot::YAxisType)ui->Y1type->currentIndex())) {
        ui->Y1type->setCurrentIndex(0);
    }
    if(!supported.count((TraceXYPlot::YAxisType)ui->Y2type->currentIndex())) {
        ui->Y2type->setCurrentIndex(0);
    }

    QString unit;
    switch(type) {
    case TraceXYPlot::XAxisType::Frequency: unit = "Hz"; break;
    case TraceXYPlot::XAxisType::Time: unit = "s"; break;
    case TraceXYPlot::XAxisType::Distance: unit = "m"; break;
    }
    ui->Xmin->setUnit(unit);
    ui->Xmax->setUnit(unit);
    ui->Xdivs->setUnit(unit);
}

QString XYplotAxisDialog::YAxisUnit(TraceXYPlot::YAxisType type)
{
    switch(type) {
    case TraceXYPlot::YAxisType::Magnitude: return "db"; break;
    case TraceXYPlot::YAxisType::Phase: return "°"; break;
    case TraceXYPlot::YAxisType::VSWR: return ""; break;
    case TraceXYPlot::YAxisType::Impulse: return ""; break;
    case TraceXYPlot::YAxisType::Step: return ""; break;
    case TraceXYPlot::YAxisType::Impedance: return "Ohm"; break;
    default: return ""; break;
    }
}

std::set<TraceXYPlot::YAxisType> XYplotAxisDialog::supportedYAxis(TraceXYPlot::XAxisType type)
{
    set<TraceXYPlot::YAxisType> ret = {TraceXYPlot::YAxisType::Disabled};
    switch(type) {
    case TraceXYPlot::XAxisType::Frequency:
        ret.insert(TraceXYPlot::YAxisType::Magnitude);
        ret.insert(TraceXYPlot::YAxisType::Phase);
        ret.insert(TraceXYPlot::YAxisType::VSWR);
        break;
    case TraceXYPlot::XAxisType::Time:
    case TraceXYPlot::XAxisType::Distance:
        ret.insert(TraceXYPlot::YAxisType::Impulse);
        ret.insert(TraceXYPlot::YAxisType::Step);
        ret.insert(TraceXYPlot::YAxisType::Impedance);
        break;
    }
    return ret;
}
