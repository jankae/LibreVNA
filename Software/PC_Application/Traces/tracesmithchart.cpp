#include "tracesmithchart.h"

#include "preferences.h"
#include "ui_smithchartdialog.h"
#include "unit.h"
#include "QFileDialog"
#include "appwindow.h"
#include "CustomWidgets/informationbox.h"

#include <QPainter>
#include <array>
#include <math.h>
#include <QDebug>
#include <QColorDialog>

using namespace std;

TraceSmithChart::TraceSmithChart(TraceModel &model, QWidget *parent)
    : TracePolar(model, parent)
{
    Z0 = 50.0;
}

nlohmann::json TraceSmithChart::toJSON()
{
    nlohmann::json j;
    j = TracePolar::toJSON();
    j["Z0"] = Z0;
    nlohmann::json jlines;
    for(auto line : constantLines) {
        jlines.push_back(line.toJSON());
    }
    j["constantLines"] = jlines;
    return j;
}

void TraceSmithChart::fromJSON(nlohmann::json j)
{
    TracePolar::fromJSON(j);
    Z0 = j.value("Z0", 50.0);
    if(j.contains("constantLines")) {
        for(auto jline : j["constantLines"]) {
            SmithChartConstantLine line;
            line.fromJSON(jline);
            constantLines.push_back(line);
        }
    }
}

void TraceSmithChart::axisSetupDialog()
{
    auto dialog = new QDialog();
    auto ui = new Ui::SmithChartDialog();
    ui->setupUi(dialog);
    if(limitToSpan) {
        ui->displayModeFreq->setCurrentIndex(1);
    } else {
        ui->displayModeFreq->setCurrentIndex(0);
    }
    if(limitToEdge) {
        ui->displayModeImp->setCurrentIndex(1);
    } else {
        ui->displayModeImp->setCurrentIndex(0);
    }

    ui->displayStartFreq->setUnit("Hz");
    ui->displayStartFreq->setPrefixes(" kMG");
    ui->displayStartFreq->setPrecision(6);
    ui->displayStartFreq->setValue(fmin);
    ui->displayStopFreq->setUnit("Hz");
    ui->displayStopFreq->setPrefixes(" kMG");
    ui->displayStopFreq->setPrecision(6);
    ui->displayStopFreq->setValue(fmax);

    connect(ui->displayFreqOverride, &QCheckBox::toggled, [=](bool active){
        ui->displayModeFreq->setEnabled(!active);
        ui->displayStartFreq->setEnabled(active);
        ui->displayStopFreq->setEnabled(active);
    });
    ui->displayFreqOverride->setChecked(manualFrequencyRange);
    emit ui->displayFreqOverride->toggled(manualFrequencyRange);

    ui->zoomReflection->setPrecision(3);
    ui->zoomFactor->setPrecision(3);
    ui->zoomReflection->setValue(edgeReflection);
    ui->zoomFactor->setValue(1.0/edgeReflection);
    ui->offsetRealAxis->setValue(dx);

    ui->impedance->setUnit("Ω");
    ui->impedance->setPrecision(3);
    ui->impedance->setValue(Z0);

    auto model = new SmithChartContantLineModel(*this);
    ui->lineTable->setModel(model);
    ui->lineTable->setItemDelegateForColumn(SmithChartContantLineModel::ColIndexType, new SmithChartTypeDelegate);
    ui->lineTable->setItemDelegateForColumn(SmithChartContantLineModel::ColIndexParam, new SmithChartParamDelegate);

    connect(ui->buttonBox, &QDialogButtonBox::accepted, [=](){
       limitToSpan = ui->displayModeFreq->currentIndex() == 1;
       limitToEdge = ui->displayModeImp->currentIndex() == 1;
       manualFrequencyRange = ui->displayFreqOverride->isChecked();
       fmin = ui->displayStartFreq->value();
       fmax = ui->displayStopFreq->value();
       updateContextMenu();
       triggerReplot();
    });
    connect(ui->zoomFactor, &SIUnitEdit::valueChanged, [=](){
        edgeReflection = 1.0 / ui->zoomFactor->value();
        ui->zoomReflection->setValueQuiet(edgeReflection);
    });
    connect(ui->zoomReflection, &SIUnitEdit::valueChanged, [=](){
        edgeReflection = ui->zoomReflection->value();
        ui->zoomFactor->setValueQuiet(1.0 / edgeReflection);
    });
    connect(ui->offsetRealAxis, &SIUnitEdit::valueChanged, [=](){
        dx = ui->offsetRealAxis->value();
    });
    connect(ui->impedance, &SIUnitEdit::valueChanged, [=](){
        Z0 = ui->impedance->value();
        for(auto t : traces) {
            if(t.second) {
                checkIfStillSupported(t.first);
            }
        }
        // depending on the preferences, the Z0 value may have been changed to match the active traces,
        // overwrite again without causing an additional signal
        ui->impedance->setValueQuiet(Z0);
    });
    connect(ui->lineTable, &QTableView::clicked, [=](const QModelIndex &index){
        if(index.column() == SmithChartContantLineModel::ColIndexColor) {
            auto line = &constantLines[index.row()];
            auto newColor = QColorDialog::getColor(line->getColor(), parentWidget(), "Select color", QColorDialog::DontUseNativeDialog);
            if(newColor.isValid()) {
                line->setColor(newColor);
                emit model->dataChanged(index, index);
                triggerReplot();
            }
        }
    });

    auto updatePersistentEditors = [=](){
        for(unsigned int i=0;i<constantLines.size();i++) {
            ui->lineTable->openPersistentEditor(model->index(i, SmithChartContantLineModel::ColIndexType));
        }
    };

    connect(ui->addLine, &QPushButton::clicked, [=](){
        model->beginResetModel();
        constantLines.push_back(SmithChartConstantLine());
        model->endResetModel();
        updatePersistentEditors();
    });
    connect(ui->removeLine, &QPushButton::clicked, [=](){
        auto selected = ui->lineTable->selectionModel()->selectedRows();
        // get indices of lines to delete
        std::vector<int> toDelete;
        for(auto s : selected) {
            toDelete.push_back(s.row());
        }
        // delete starting with highest index (this makes sure that indices are not messed up after deleting an element
        std::sort(toDelete.begin(), toDelete.end());
        model->beginResetModel();
        for (auto i = toDelete.rbegin(); i != toDelete.rend(); i++) {
            constantLines.erase(constantLines.begin() + *i);
        }
        model->endResetModel();
        updatePersistentEditors();
    });

    updatePersistentEditors();
    if(AppWindow::showGUI()) {
        dialog->show();
    }
}

bool TraceSmithChart::configureForTrace(Trace *t)
{
    if(dropSupported(t)) {
        Z0 = t->getReferenceImpedance();
        for(auto t : traces) {
            if(t.second && t.first->getReferenceImpedance() != Z0) {
                enableTrace(t.first, false);
            }
        }
        updateContextMenu();
        return true;
    }
    return false;
}

void TraceSmithChart::draw(QPainter &p) {
    auto pref = Preferences::getInstance();

    // translate coordinate system so that the smith chart sits in the origin and has a size of 1
    auto w = p.window();
    p.save();
    p.translate(w.width()/2, w.height()/2);
    auto scale = qMin(w.height(), w.width()) / (2.0 * polarCoordMax);
    p.scale(scale, scale);

    transform = p.transform();
    p.restore();

    auto minimumVisibleFrequency = std::numeric_limits<double>::lowest();
    auto maximumVisibleFrequency = std::numeric_limits<double>::max();
    if(manualFrequencyRange) {
        minimumVisibleFrequency = fmin;
        maximumVisibleFrequency = fmax;
    } else if(limitToSpan) {
        minimumVisibleFrequency = sweep_fmin;
        maximumVisibleFrequency = sweep_fmax;
    }

    auto drawArc = [&](SmithChartArc a) {
        a.constrainToCircle(QPointF(0,0), edgeReflection);
        auto topleft = dataToPixel(complex<double>(a.center.x() - a.radius, a.center.y() - a.radius));
        auto bottomright = dataToPixel(complex<double>(a.center.x() + a.radius, a.center.y() + a.radius));
        a.startAngle *= 5760 / (2*M_PI);
        a.spanAngle *= 5760 / (2*M_PI);
        p.drawArc(QRect(topleft, bottomright), a.startAngle, a.spanAngle);
    };

    // Outer circle
    auto pen = QPen(pref.Graphs.Color.axis);
    pen.setCosmetic(true);
    p.setPen(pen);
    drawArc(SmithChartArc(QPointF(0, 0), edgeReflection, 0, 2*M_PI));

    constexpr int Circles = 6;
    pen = QPen(pref.Graphs.Color.Ticks.divisions, 0.5, Qt::DashLine);
    pen.setCosmetic(true);
    p.setPen(pen);
    for(int i=1;i<Circles * 2;i++) {
        auto radius = (double) i / Circles;
        drawArc(SmithChartArc(QPointF(1.0 - radius+dx, 0.0), radius, 0, 2*M_PI));
        drawArc(SmithChartArc(QPointF(1.0 + radius+dx, 0.0), radius, 0, 2*M_PI));
    }

      p.drawLine(dataToPixel(complex<double>(edgeReflection,0)),dataToPixel(complex<double>(-edgeReflection,0)));
    constexpr std::array<double, 5> impedanceLines = {10, 25, 50, 100, 250};
    for(auto z : impedanceLines) {
        z /= Z0;
        auto radius = 1.0/z;
        drawArc(SmithChartArc(QPointF(1.0+dx, radius), radius, 0, 2*M_PI));
        drawArc(SmithChartArc(QPointF(1.0+dx, -radius), radius, 0, 2*M_PI));
    }

    // draw custom constant parameter lines
    for(auto line : constantLines) {
        pen = QPen(line.getColor(), pref.Graphs.lineWidth);
        pen.setCosmetic(true);
        p.setPen(pen);
        for(auto arc : line.getArcs(Z0)) {
            drawArc(arc);
        }
    }

    for(auto t : traces) {
        if(!t.second) {
            // trace not enabled in plot
            continue;
        }
        auto trace = t.first;
        if(!trace->isVisible()) {
            // trace marked invisible
            continue;
        }
        pen = QPen(trace->color(), pref.Graphs.lineWidth);
        pen.setCosmetic(true);
        p.setPen(pen);
        int nPoints = trace->size();
        for(int i=1;i<nPoints;i++) {
            auto last = trace->sample(i-1);
            auto now = trace->sample(i);
            if ((trace->getDataType() == Trace::DataType::Frequency) && (last.x < minimumVisibleFrequency || now.x > maximumVisibleFrequency)) {
                continue;
            }
            if(isnan(now.y.real())) {
                break;
            }

            last = dataAddDx(last);
            now = dataAddDx(now);

            // scale to size of smith diagram
            QPointF p1 = dataToPixel(last);
            QPointF p2 = dataToPixel(now);

            if(limitToEdge && (abs(last.y) > edgeReflection || abs(now.y) > edgeReflection)) {
                // partially outside of visible area, constrain
                if(!TracePolar::constrainLineToCircle(p1, p2, transform.map(QPointF(0,0)), polarCoordMax * scale)) {
                    // completely out of visible area
                    continue;
                }
            }

            // draw line
            p.drawLine(p1, p2);
        }
        if(trace->size() > 0) {
            // only draw markers if the trace has at least one point
            auto markers = t.first->getMarkers();
            for(auto m : markers) {
                if(!m->isVisible()) {
                    continue;
                }
//                if (m->isTimeDomain()) {
//                    continue;
//                }
                if (m->getPosition() < minimumVisibleFrequency || m->getPosition() > maximumVisibleFrequency) {
                    continue;
                }
                if(m->getPosition() < trace->minX() || m->getPosition() > trace->maxX()) {
                    // marker not in trace range
                    continue;
                }
                auto coords = m->getData();
                coords = dataAddDx(coords);

                if (limitToEdge && abs(coords) > edgeReflection) {
                    // outside of visible area
                    continue;
                }
                auto point = dataToPixel(coords);
                auto symbol = m->getSymbol();
                p.drawPixmap(point.x() - symbol.width()/2, point.y() - symbol.height(), symbol);
            }
        }
    }
    if(dropPending) {
        // adjust coords due to shifted restore
        p.setOpacity(0.5);
        p.setBrush(Qt::white);
        p.setPen(Qt::white);
        p.drawEllipse(-polarCoordMax, -polarCoordMax, 2*polarCoordMax, 2*polarCoordMax);
        auto font = p.font();
        font.setPixelSize(20);
        p.setFont(font);
        p.setOpacity(1.0);
        p.setPen(Qt::white);
        auto text = "Drop here to add\n" + dropTrace->name() + "\nto Smith chart";
        p.drawText(p.window(), Qt::AlignCenter, text);
    } else {
    }
}

void TraceSmithChart::traceDropped(Trace *t, QPoint position)
{
    if(!supported(t) && dropSupported(t)) {
        // needs to switch to a different domain for the graph
        if(!InformationBox::AskQuestion("Reference impedance change", "You dropped a trace that is not supported with the currently selected reference impedance."
                                    " Do you want to remove all traces and change the graph to the correct reference imppedance?", true, "ReferenceImpedanceChangeRequest")) {
            // user declined to change domain, to not add change impedance
            return;
        }
        // attempt to configure for this trace
        configureForTrace(t);
    }
    TracePlot::traceDropped(t, position);
}

bool TraceSmithChart::dropSupported(Trace *t)
{
    if(!t->isReflection()) {
        return false;
    }
    switch(t->outputType()) {
    case Trace::DataType::Frequency:
    case Trace::DataType::Power:
    case Trace::DataType::TimeZeroSpan:
        return true;
    default:
        return false;
    }
}

QString TraceSmithChart::mouseText(QPoint pos)
{
    auto dataDx = pixelToData(pos);
    if(abs(dataDx) <= edgeReflection) {
        auto data = complex<double>(dataDx.real()-dx, dataDx.imag());
        data = Z0 * (1.0 + data) / (1.0 - data);
        auto ret = Unit::ToString(data.real(), "", " ", 3);
        if(data.imag() >= 0) {
            ret += "+";
        }
        ret += Unit::ToString(data.imag(), "j", " ", 3);
        return ret;
    } else {
        return QString();
    }
}

bool TraceSmithChart::supported(Trace *t)
{
    if(t->getReferenceImpedance() != Z0) {
        return false;
    }
    return dropSupported(t);
}

SmithChartArc::SmithChartArc(QPointF center, double radius, double startAngle, double spanAngle)
    : center(center),
      radius(radius),
      startAngle(startAngle),
      spanAngle(spanAngle)
{

}

void SmithChartArc::constrainToCircle(QPointF center, double radius)
{
    // check if arc/circle intersect
    auto centerDiff = this->center - center;
    auto centerDistSquared = centerDiff.x() * centerDiff.x() + centerDiff.y() * centerDiff.y();
    if (centerDistSquared >= (radius + this->radius) * (radius + this->radius)) {
        // arc completely outside of constraining circle
        spanAngle = 0.0;
        return;
    } else if (centerDistSquared <= (radius - this->radius) * (radius - this->radius)) {
        if (radius >= this->radius) {
            // arc completely in constraining circle, do nothing
            return;
        } else {
            // arc completely outside of circle
            spanAngle = 0.0;
            return;
        }
    } else {
        // there are intersections between the arc and the circle. Calculate points according to https://stackoverflow.com/questions/3349125/circle-circle-intersection-points
        auto distance = sqrt(centerDistSquared);
        auto a = (this->radius*this->radius-radius*radius+distance*distance) / (2*distance);
        auto h = sqrt(this->radius*this->radius - a*a);
        auto intersectMiddle = this->center + a*(center - this->center) / distance;
        auto rotatedCenterDiff = center - this->center;
        swap(rotatedCenterDiff.rx(), rotatedCenterDiff.ry());
        rotatedCenterDiff.setY(-rotatedCenterDiff.y());
        auto intersect1 = intersectMiddle + h * rotatedCenterDiff / distance;
        auto intersect2 = intersectMiddle - h * rotatedCenterDiff / distance;

        // got intersection points, convert into angles from arc center
        auto wrapAngle = [](double angle) -> double {
            double ret = fmod(angle, 2*M_PI);
            if(ret < 0) {
                ret += 2*M_PI;
            }
            return ret;
        };

        auto angle1 = wrapAngle(atan2((intersect1 - this->center).y(), (intersect1 - this->center).x()));
        auto angle2 = wrapAngle(atan2((intersect2 - this->center).y(), (intersect2 - this->center).x()));

        auto angleDiff = wrapAngle(angle2 - angle1);
        if ((angleDiff >= M_PI) ^ (a > 0.0)) {
            // allowed angles go from intersect1 to intersect2
            if(startAngle < angle1) {
                startAngle = angle1;
            }
            auto maxSpan = wrapAngle(angle2 - startAngle);
            if(spanAngle > maxSpan) {
                spanAngle = maxSpan;
            }
        } else {
            // allowed angles go from intersect2 to intersect1
            if(startAngle < angle2) {
                startAngle = angle2;
            }
            auto maxSpan = wrapAngle(angle1 - startAngle);
            if(spanAngle > maxSpan) {
                spanAngle = maxSpan;
            }
        }
    }
}

SmithChartConstantLine::SmithChartConstantLine()
{
    type = Type::VSWR;
    param = 10.0;
    color = Qt::darkRed;
}

std::vector<SmithChartArc> SmithChartConstantLine::getArcs(double Z0)
{
    std::vector<SmithChartArc> arcs;
    switch(type) {
    case Type::VSWR:
        arcs.push_back(SmithChartArc(QPointF(0.0, 0.0), (param - 1.0) / (param + 1.0)));
        break;
    case Type::Resistance: {
        auto circleLeft = (param / Z0 - 1.0) / (param / Z0 + 1.0);
        arcs.push_back(SmithChartArc(QPointF((circleLeft + 1.0) / 2, 0.0), (1.0 - circleLeft) / 2.0));
    }
        break;
    case Type::Reactance: {
        auto radius = 1.0/(param / Z0);
        if(radius > 0) {
            arcs.push_back(SmithChartArc(QPointF(1.0, radius), radius));
        } else {
            arcs.push_back(SmithChartArc(QPointF(1.0, radius), -radius));
        }
    }
        break;
    case Type::Q: {
        auto center = 1.0 / param;
        auto radius = sqrt(center*center + 1.0);
        arcs.push_back(SmithChartArc(QPointF(0.0, center), radius));
        arcs.push_back(SmithChartArc(QPointF(0.0, -center), radius));
    }
        break;
    case Type::Last:
        break;
    }
    return arcs;
}

QColor SmithChartConstantLine::getColor() const
{
    return color;
}

void SmithChartConstantLine::fromJSON(nlohmann::json j)
{
    type = TypeFromString(QString::fromStdString(j.value("type", "VSWR")));
    if(type == Type::Last) {
        type = Type::VSWR;
    }
    param = j.value("param", 1.0);
    color = QColor(QString::fromStdString(j.value("color", "red")));
}

nlohmann::json SmithChartConstantLine::toJSON()
{
    nlohmann::json j;
    j["type"] = TypeToString(type).toStdString();
    j["param"] = param;
    j["color"] = color.name().toStdString();
    return j;
}

QString SmithChartConstantLine::getParamUnit()
{
    switch(type) {
    case Type::VSWR: return "";
    case Type::Resistance: return "Ω";
    case Type::Reactance: return "Ωj";
    case Type::Q: return "";
    case Type::Last: break;
    }
    return "";
}

QString SmithChartConstantLine::TypeToString(SmithChartConstantLine::Type type)
{
    switch(type) {
    case Type::VSWR: return "VSWR";
    case Type::Resistance: return "Resistance";
    case Type::Reactance: return "Reactance";
    case Type::Q: return "Q";
    case Type::Last:break;
    }
    // should never get here
    return "Invalid";
}

SmithChartConstantLine::Type SmithChartConstantLine::TypeFromString(QString s)
{
    for(unsigned int i=0;i<(unsigned int)Type::Last;i++) {
        if(TypeToString((Type) i) == s) {
            return (Type) i;
        }
    }
    return Type::Last;
}

void SmithChartConstantLine::setColor(const QColor &value)
{
    color = value;
}

double SmithChartConstantLine::getParam() const
{
    return param;
}

void SmithChartConstantLine::setParam(double value)
{
    param = value;
}

void SmithChartConstantLine::setType(const Type &value)
{
    type = value;
}

SmithChartConstantLine::Type SmithChartConstantLine::getType() const
{
    return type;
}

static constexpr int rowHeight = 21;

QSize SmithChartParamDelegate::sizeHint(const QStyleOptionViewItem &, const QModelIndex &) const
{
    return QSize(0, rowHeight);
}

QWidget *SmithChartParamDelegate::createEditor(QWidget *parent, const QStyleOptionViewItem &, const QModelIndex &index) const
{
    auto line = static_cast<const SmithChartContantLineModel*>(index.model())->lineFromIndex(index);
    auto editor = new SIUnitEdit(line->getParamUnit(), "pnum kMG", 6);
    editor->setValue(line->getParam());
    editor->setMaximumHeight(rowHeight);
    editor->setParent(parent);
    connect(editor, &SIUnitEdit::valueUpdated, this, &SmithChartParamDelegate::commitData);
    return editor;
}

void SmithChartParamDelegate::setModelData(QWidget *editor, QAbstractItemModel *model, const QModelIndex &index) const
{
    auto line = ((SmithChartContantLineModel*)model)->lineFromIndex(index);
    auto si = (SIUnitEdit*) editor;
    line->setParam(si->value());
}

QSize SmithChartTypeDelegate::sizeHint(const QStyleOptionViewItem &, const QModelIndex &) const
{
    return QSize(0, rowHeight);
}

QWidget *SmithChartTypeDelegate::createEditor(QWidget *parent, const QStyleOptionViewItem &, const QModelIndex &index) const
{
    auto line = static_cast<const SmithChartContantLineModel*>(index.model())->lineFromIndex(index);
    auto editor = new QComboBox();
    for(unsigned int i=0;i<(unsigned int)SmithChartConstantLine::Type::Last;i++) {
        editor->addItem(SmithChartConstantLine::TypeToString((SmithChartConstantLine::Type) i));
    }
    editor->setCurrentIndex((int) line->getType());
    connect(editor, qOverload<int>(&QComboBox::currentIndexChanged), [=](int) {
        emit const_cast<SmithChartTypeDelegate*>(this)->commitData(editor);
    });
    editor->setMaximumHeight(rowHeight);
    editor->setParent(parent);
    return editor;
}

void SmithChartTypeDelegate::setModelData(QWidget *editor, QAbstractItemModel *model, const QModelIndex &index) const
{
    auto line = ((SmithChartContantLineModel*)model)->lineFromIndex(index);
    auto *cb = (QComboBox*) editor;
    line->setType((SmithChartConstantLine::Type) cb->currentIndex());
    // parameter unit may have changed, update model
    auto paramIndex = model->index(index.row(), SmithChartContantLineModel::ColIndexParam);
    emit model->dataChanged(paramIndex, paramIndex);
}

SmithChartContantLineModel::SmithChartContantLineModel(TraceSmithChart &chart, QObject *parent)
    : chart(chart)
{
    Q_UNUSED(parent);
}

int SmithChartContantLineModel::rowCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent);
    return chart.constantLines.size();
}

int SmithChartContantLineModel::columnCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent);
    return ColIndexLast;
}

QVariant SmithChartContantLineModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid())
        return QVariant();

    if ((unsigned int) index.row() >= chart.constantLines.size())
        return QVariant();

    auto line = chart.constantLines[index.row()];
    switch(index.column()) {
    case ColIndexColor:
        if (role == Qt::BackgroundColorRole) {
            return line.getColor();
        }
        break;
    case ColIndexType:
        if (role == Qt::DisplayRole) {
            return SmithChartConstantLine::TypeToString(line.getType());
        }
        break;
    case ColIndexParam:
        if (role == Qt::DisplayRole) {
            return Unit::ToString(line.getParam(), line.getParamUnit(), "pnum kMG", 6);
        }
        break;
    default:
        break;
    }
    return QVariant();
}

QVariant SmithChartContantLineModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    Q_UNUSED(orientation);
    if(role == Qt::DisplayRole) {
        switch(section) {
        case ColIndexColor: return "Color";
        case ColIndexType: return "Type";
        case ColIndexParam: return "Parameter";
        }
    }
    return QVariant();
}

Qt::ItemFlags SmithChartContantLineModel::flags(const QModelIndex &index) const
{
    Qt::ItemFlags flags = Qt::ItemIsSelectable | Qt::ItemIsEnabled;
    switch(index.column()) {
    case ColIndexType:
    case ColIndexParam:
        flags |= Qt::ItemIsEditable;
        break;
    }
    return flags;
}

SmithChartConstantLine* SmithChartContantLineModel::lineFromIndex(const QModelIndex &index) const
{
    if(index.isValid() && index.row() < (int) chart.constantLines.size()) {
        return &chart.constantLines[index.row()];
    } else {
        return nullptr;
    }
}
