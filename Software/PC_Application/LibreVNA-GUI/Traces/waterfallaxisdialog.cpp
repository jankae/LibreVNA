#include "waterfallaxisdialog.h"

#include "ui_waterfallaxisdialog.h"

#include <QStandardItemModel>
#include <QPushButton>

using namespace std;

static void enableComboBoxItem(QComboBox *cb, int itemNum, bool enable) {
    auto *model = qobject_cast<QStandardItemModel *>(cb->model());
    auto item = model->item(itemNum);
    item->setFlags(enable ? item->flags() | Qt::ItemIsEnabled
                            : item->flags() & ~Qt::ItemIsEnabled);
}

WaterfallAxisDialog::WaterfallAxisDialog(TraceWaterfall *plot) :
    QDialog(nullptr),
    ui(new Ui::WaterfallAxisDialog),
    plot(plot)
{
    ui->setupUi(this);
    ui->Wtype->clear();
    ui->Wtype->setMaxVisibleItems(20);

    for(int i=0;i<(int) YAxis::Type::Last;i++) {
        ui->Wtype->addItem(YAxis::TypeToName((YAxis::Type) i));
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

    for(unsigned int i=0;i<(int) TraceWaterfall::Alignment::Last;i++) {
        ui->Xalignment->addItem(TraceWaterfall::AlignmentToString((TraceWaterfall::Alignment) i));
    }

    // Setup GUI connections
    connect(ui->Wtype, qOverload<int>(&QComboBox::currentIndexChanged), [=](int index) {
       //ui->Y1log->setEnabled(index != 0);
       ui->Wlinear->setEnabled(index != 0);
       ui->Wauto->setEnabled(index != 0);
       bool autoRange = ui->Wauto->isChecked();
       ui->Wmin->setEnabled(index != 0 && !autoRange);
       ui->Wmax->setEnabled(index != 0 && !autoRange);
       auto type = (YAxis::Type) index;
       QString unit = YAxis::Unit(type, plot->getModel().getSource());
       QString prefixes = YAxis::Prefixes(type);
       ui->Wmin->setUnit(unit);
       ui->Wmin->setPrefixes(prefixes);
       ui->Wmax->setUnit(unit);
       ui->Wmax->setPrefixes(prefixes);
       ui->Wmin->setValue(YAxis::getDefaultLimitMin(type));
       ui->Wmax->setValue(YAxis::getDefaultLimitMax(type));
    });
    connect(ui->Wauto, &QCheckBox::toggled, [this](bool checked) {
       ui->Wmin->setEnabled(!checked);
       ui->Wmax->setEnabled(!checked);
    });
    connect(ui->Wmode, qOverload<int>(&QComboBox::currentIndexChanged), [this](int index) {
       ui->WmaxSweeps->setEnabled(index == 1);
    });

    ui->XType->setCurrentIndex((int) plot->xAxis.getType());

    ui->Wmin->setPrefixes("pnum kMG");
    ui->Wmax->setPrefixes("pnum kMG");

    XAxisTypeChanged((int) plot->xAxis.getType());
    connect(ui->XType, qOverload<int>(&QComboBox::currentIndexChanged), this, &WaterfallAxisDialog::XAxisTypeChanged);

    // Fill initial values
    ui->Wtype->setCurrentIndex((int) plot->yAxis.getType());
    if(plot->yAxis.getLog()) {
        ui->Wlog->setChecked(true);
    } else {
        ui->Wlinear->setChecked(true);
    }
    ui->Wauto->setChecked(plot->yAxis.getAutorange());
    ui->Wmin->setValueQuiet(plot->yAxis.getRangeMin());
    ui->Wmax->setValueQuiet(plot->yAxis.getRangeMax());
    if(plot->dir == TraceWaterfall::Direction::TopToBottom) {
        ui->Wdir->setCurrentIndex(0);
    } else {
        ui->Wdir->setCurrentIndex(1);
    }
    ui->Wpixels->setValue(plot->pixelsPerLine);
    ui->Wmode->setCurrentIndex(plot->keepDataBeyondPlotSize ? 1 : 0);
    ui->WmaxSweeps->setValue(plot->maxDataSweeps);

    if(plot->xAxis.getLog()) {
        ui->Xlog->setChecked(true);
    } else {
        ui->Xlinear->setChecked(true);
    }
    ui->Xalignment->setCurrentIndex((int) plot->align);

    connect(ui->buttonBox->button(QDialogButtonBox::Apply), &QPushButton::clicked, [=](){
       on_buttonBox_accepted();
    });
}

WaterfallAxisDialog::~WaterfallAxisDialog()
{
    delete ui;
}

void WaterfallAxisDialog::on_buttonBox_accepted()
{
    // set plot values to the ones selected in the dialog
    plot->xAxis.set(plot->xAxis.getType(), ui->Xlog->isChecked(), true, plot->xAxis.getRangeMin(), plot->xAxis.getRangeMax(), 10, false);
    plot->yAxis.set((YAxis::Type) ui->Wtype->currentIndex(), ui->Wlog->isChecked(), ui->Wauto->isChecked(), ui->Wmin->value(), ui->Wmax->value(), 2, false);
    if(ui->Wdir->currentIndex() == 0) {
        plot->dir = TraceWaterfall::Direction::TopToBottom;
    } else {
        plot->dir = TraceWaterfall::Direction::BottomToTop;
    }
    plot->pixelsPerLine = ui->Wpixels->value();
    plot->maxDataSweeps = ui->WmaxSweeps->value();
    if(ui->Wmode->currentIndex() == 0) {
        plot->keepDataBeyondPlotSize = false;
    } else {
        plot->keepDataBeyondPlotSize = true;
    }
    plot->align = (TraceWaterfall::Alignment) ui->Xalignment->currentIndex();
    plot->updateYAxis();
}

void WaterfallAxisDialog::XAxisTypeChanged(int XAxisIndex)
{
    auto type = (XAxis::Type) XAxisIndex;
    auto supported = supportedYAxis(type);
    for(unsigned int i=0;i<(int) YAxis::Type::Last;i++) {
        auto t = (YAxis::Type) i;
        auto enable = supported.count(t) > 0;
        auto index = (int) t;
        enableComboBoxItem(ui->Wtype, index, enable);
    }
    // Disable Yaxis if previously selected type is not supported
    if(!supported.count((YAxis::Type)ui->Wtype->currentIndex())) {
        ui->Wtype->setCurrentIndex(0);
    }

    if(type == XAxis::Type::Frequency) {
        ui->Xlog->setEnabled(true);
    } else {
        // log option only available for frequency axis
        if(ui->Xlog->isChecked()) {
            ui->Xlinear->setChecked(true);
        }
        ui->Xlog->setEnabled(false);
    }
}

std::set<YAxis::Type> WaterfallAxisDialog::supportedYAxis(XAxis::Type type)
{
    set<YAxis::Type> ret = {YAxis::Type::Disabled};
    auto source = plot->getModel().getSource();
    if(source == TraceModel::DataSource::VNA) {
        switch(type) {
        case XAxis::Type::Frequency:
        case XAxis::Type::Power:
            ret.insert(YAxis::Type::Magnitude);
            ret.insert(YAxis::Type::MagnitudeLinear);
            ret.insert(YAxis::Type::Phase);
            ret.insert(YAxis::Type::VSWR);
            ret.insert(YAxis::Type::Real);
            ret.insert(YAxis::Type::Imaginary);
            ret.insert(YAxis::Type::SeriesR);
            ret.insert(YAxis::Type::Reactance);
            ret.insert(YAxis::Type::Capacitance);
            ret.insert(YAxis::Type::Inductance);
            ret.insert(YAxis::Type::QualityFactor);
            break;
        case XAxis::Type::Time:
        case XAxis::Type::Distance:
            ret.insert(YAxis::Type::ImpulseReal);
            ret.insert(YAxis::Type::ImpulseMag);
            ret.insert(YAxis::Type::Step);
            ret.insert(YAxis::Type::Impedance);
            break;
        default:
            break;
        }
    } else if(source == TraceModel::DataSource::SA) {
        switch(type) {
        case XAxis::Type::Frequency:
            ret.insert(YAxis::Type::Magnitude);
            ret.insert(YAxis::Type::MagnitudedBuV);
            break;
        default:
            break;
        }
    }
    return ret;
}

bool WaterfallAxisDialog::isSupported(XAxis::Type type)
{
    auto source = plot->getModel().getSource();
    if(source == TraceModel::DataSource::VNA) {
        // all X axis types are supported
        return true;
    } else if(source == TraceModel::DataSource::SA) {
        if (type == XAxis::Type::Frequency) {
            return true;
        } else {
            return false;
        }
    }
    return false;
}
