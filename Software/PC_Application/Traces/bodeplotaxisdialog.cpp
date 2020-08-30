#include "bodeplotaxisdialog.h"
#include "ui_bodeplotaxisdialog.h"

BodeplotAxisDialog::BodeplotAxisDialog(TraceBodePlot *plot) :
    QDialog(nullptr),
    ui(new Ui::BodeplotAxisDialog),
    plot(plot)
{
    ui->setupUi(this);

    // Setup GUI connections
    connect(ui->Y1type, qOverload<int>(&QComboBox::currentIndexChanged), [this](int index) {
       ui->Y1log->setEnabled(index != 0);
       ui->Y1linear->setEnabled(index != 0);
       ui->Y1auto->setEnabled(index != 0);
       bool autoRange = ui->Y1auto->isChecked();
       ui->Y1min->setEnabled(index != 0 && !autoRange);
       ui->Y1max->setEnabled(index != 0 && !autoRange);
       ui->Y1divs->setEnabled(index != 0 && !autoRange);
       auto type = (TraceBodePlot::YAxisType) index;
       QString unit;
       switch(type) {
       case TraceBodePlot::YAxisType::Magnitude: unit = "db"; break;
       case TraceBodePlot::YAxisType::Phase: unit = "°"; break;
       case TraceBodePlot::YAxisType::VSWR: unit = ""; break;
       default: unit = ""; break;
       }
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
       ui->Y2log->setEnabled(index != 0);
       ui->Y2linear->setEnabled(index != 0);
       ui->Y2auto->setEnabled(index != 0);
       bool autoRange = ui->Y2auto->isChecked();
       ui->Y2min->setEnabled(index != 0 && !autoRange);
       ui->Y2max->setEnabled(index != 0 && !autoRange);
       ui->Y2divs->setEnabled(index != 0 && !autoRange);
       auto type = (TraceBodePlot::YAxisType) index;
       QString unit;
       switch(type) {
       case TraceBodePlot::YAxisType::Magnitude: unit = "db"; break;
       case TraceBodePlot::YAxisType::Phase: unit = "°"; break;
       case TraceBodePlot::YAxisType::VSWR: unit = ""; break;
       default: unit = ""; break;
       }
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
    });

    ui->Xmin->setUnit("Hz");
    ui->Xmax->setUnit("Hz");
    ui->Xdivs->setUnit("Hz");
    ui->Xmin->setPrefixes(" kMG");
    ui->Xmax->setPrefixes(" kMG");
    ui->Xdivs->setPrefixes(" kMG");

    // Fill initial values
    // assume same order in YAxisType enum as in ComboBox items
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

    ui->Xauto->setChecked(plot->XAxis.autorange);
    ui->Xmin->setValueQuiet(plot->XAxis.rangeMin);
    ui->Xmax->setValueQuiet(plot->XAxis.rangeMax);
    ui->Xdivs->setValueQuiet(plot->XAxis.rangeDiv);
}

BodeplotAxisDialog::~BodeplotAxisDialog()
{
    delete ui;
}

void BodeplotAxisDialog::on_buttonBox_accepted()
{
    // set plot values to the ones selected in the dialog
    plot->setYAxis(0, (TraceBodePlot::YAxisType) ui->Y1type->currentIndex(), ui->Y1log->isChecked(), ui->Y1auto->isChecked(), ui->Y1min->value(), ui->Y1max->value(), ui->Y1divs->value());
    plot->setYAxis(1, (TraceBodePlot::YAxisType) ui->Y2type->currentIndex(), ui->Y2log->isChecked(), ui->Y2auto->isChecked(), ui->Y2min->value(), ui->Y2max->value(), ui->Y2divs->value());
    plot->setXAxis(ui->Xauto->isChecked(), ui->Xmin->value(), ui->Xmax->value(), ui->Xdivs->value());
}
