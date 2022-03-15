#include "tracesmithchart.h"

#include "Marker/marker.h"
#include "preferences.h"
#include "ui_smithchartdialog.h"
#include "unit.h"
#include "QFileDialog"
#include "Util/util.h"
#include "appwindow.h"

#include <QPainter>
#include <array>
#include <math.h>
#include <QDebug>
#include <QColorDialog>

using namespace std;

TraceSmithChart::TraceSmithChart(TraceModel &model, QWidget *parent)
    : TracePlot(model, parent)
{
    limitToSpan = true;
    edgeReflection = 1.0;
    initializeTraceInfo();
}

nlohmann::json TraceSmithChart::toJSON()
{
    nlohmann::json j;
    j["limit_to_span"] = limitToSpan;
    j["limit_to_edge"] = limitToEdge;
    j["edge_reflection"] = edgeReflection;
    nlohmann::json jtraces;
    for(auto t : traces) {
        if(t.second) {
            jtraces.push_back(t.first->toHash());
        }
    }
    j["traces"] = jtraces;
    nlohmann::json jlines;
    for(auto line : constantLines) {
        jlines.push_back(line.toJSON());
    }
    j["constantLines"] = jlines;
    return j;
}

void TraceSmithChart::fromJSON(nlohmann::json j)
{
    limitToSpan = j.value("limit_to_span", true);
    limitToEdge = j.value("limit_to_edge", false);
    edgeReflection = j.value("edge_reflection", 1.0);
    for(unsigned int hash : j["traces"]) {
        // attempt to find the traces with this hash
        bool found = false;
        for(auto t : model.getTraces()) {
            if(t->toHash() == hash) {
                enableTrace(t, true);
                found = true;
                break;
            }
        }
        if(!found) {
            qWarning() << "Unable to find trace with hash" << hash;
        }
    }
    if(j.contains("constantLines")) {
        for(auto jline : j["constantLines"]) {
            SmithChartConstantLine line;
            line.fromJSON(jline);
            constantLines.push_back(line);
        }
    }
}

void TraceSmithChart::wheelEvent(QWheelEvent *event)
{
    // most mousewheel have 15 degree increments, the reported delta is in 1/8th degree -> 120
    auto increment = event->angleDelta().y() / 120.0;
    // round toward bigger step in case of special higher resolution mousewheel
    int steps = increment > 0 ? ceil(increment) : floor(increment);

    constexpr double zoomfactor = 1.1;
    auto zoom = pow(zoomfactor, steps);
    edgeReflection /= zoom;
    triggerReplot();
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
    ui->zoomReflection->setPrecision(3);
    ui->zoomFactor->setPrecision(3);
    ui->zoomReflection->setValue(edgeReflection);
    ui->zoomFactor->setValue(1.0/edgeReflection);

    auto model = new SmithChartContantLineModel(*this);
    ui->lineTable->setModel(model);
    ui->lineTable->setItemDelegateForColumn(SmithChartContantLineModel::ColIndexType, new SmithChartTypeDelegate);
    ui->lineTable->setItemDelegateForColumn(SmithChartContantLineModel::ColIndexParam, new SmithChartParamDelegate);

    connect(ui->buttonBox, &QDialogButtonBox::accepted, [=](){
       limitToSpan = ui->displayModeFreq->currentIndex() == 1;
       limitToEdge = ui->displayModeImp->currentIndex() == 1;
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

QPoint TraceSmithChart::dataToPixel(std::complex<double> d)
{
    return transform.map(QPoint(d.real() * smithCoordMax * (1.0 / edgeReflection), -d.imag() * smithCoordMax * (1.0 / edgeReflection)));
}

QPoint TraceSmithChart::dataToPixel(Trace::Data d)
{
    if(d.x < sweep_fmin || d.x > sweep_fmax) {
        return QPoint();
    }
    return dataToPixel(d.y);
}

std::complex<double> TraceSmithChart::pixelToData(QPoint p)
{
    auto data = transform.inverted().map(QPointF(p));
    return complex<double>(data.x() / smithCoordMax * edgeReflection, -data.y() / smithCoordMax * edgeReflection);
}

QPoint TraceSmithChart::markerToPixel(Marker *m)
{
    QPoint ret = QPoint();
//    if(!m->isTimeDomain()) {
        if(m->getPosition() >= sweep_fmin && m->getPosition() <= sweep_fmax) {
            auto d = m->getData();
            ret = dataToPixel(d);
        }
//    }
    return ret;
}

double TraceSmithChart::nearestTracePoint(Trace *t, QPoint pixel, double *distance)
{
    double closestDistance = numeric_limits<double>::max();
    double closestXpos = 0;
    unsigned int closestIndex = 0;
    auto samples = t->size();
    for(unsigned int i=0;i<samples;i++) {
        auto data = t->sample(i);
        auto plotPoint = dataToPixel(data);
        if (plotPoint.isNull()) {
            // destination point outside of currently displayed range
            continue;
        }
        auto diff = plotPoint - pixel;
        unsigned int distance = diff.x() * diff.x() + diff.y() * diff.y();
        if(distance < closestDistance) {
            closestDistance = distance;
            closestXpos = t->sample(i).x;
            closestIndex = i;
        }
    }
    closestDistance = sqrt(closestDistance);
    if(closestIndex > 0) {
        auto l1 = dataToPixel(t->sample(closestIndex-1));
        auto l2 = dataToPixel(t->sample(closestIndex));
        double ratio;
        auto distance = Util::distanceToLine(pixel, l1, l2, nullptr, &ratio);
        if(distance < closestDistance) {
            closestDistance = distance;
            closestXpos = t->sample(closestIndex-1).x + (t->sample(closestIndex).x - t->sample(closestIndex-1).x) * ratio;
        }
    }
    if(closestIndex < t->size() - 1) {
        auto l1 = dataToPixel(t->sample(closestIndex));
        auto l2 = dataToPixel(t->sample(closestIndex+1));
        double ratio;
        auto distance = Util::distanceToLine(pixel, l1, l2, nullptr, &ratio);
        if(distance < closestDistance) {
            closestDistance = distance;
            closestXpos = t->sample(closestIndex).x + (t->sample(closestIndex+1).x - t->sample(closestIndex).x) * ratio;
        }
    }
    if(distance) {
        *distance = closestDistance;
    }
    return closestXpos;
}

bool TraceSmithChart::xCoordinateVisible(double x)
{
    if(limitToSpan) {
        if(x >= sweep_fmin && x <= sweep_fmax) {
            return true;
        } else {
            return false;
        }
    } else {
        // complete traces visible
        return true;
    }
}

void TraceSmithChart::draw(QPainter &p) {
    auto pref = Preferences::getInstance();

    // translate coordinate system so that the smith chart sits in the origin and has a size of 1
    auto w = p.window();
    p.save();
    p.translate(w.width()/2, w.height()/2);
    auto scale = qMin(w.height(), w.width()) / (2.0 * smithCoordMax);
    p.scale(scale, scale);

    transform = p.transform();
    p.restore();

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
        drawArc(SmithChartArc(QPointF(1.0 - radius, 0.0), radius, 0, 2*M_PI));
        drawArc(SmithChartArc(QPointF(1.0 + radius, 0.0), radius, 0, 2*M_PI));
    }

      p.drawLine(dataToPixel(complex<double>(edgeReflection,0)),dataToPixel(complex<double>(-edgeReflection,0)));
    constexpr std::array<double, 5> impedanceLines = {10, 25, 50, 100, 250};
    for(auto z : impedanceLines) {
        z /= Preferences::getInstance().Acquisition.refImp;
        auto radius = 1.0/z;
        drawArc(SmithChartArc(QPointF(1.0, radius), radius, 0, 2*M_PI));
        drawArc(SmithChartArc(QPointF(1.0, -radius), radius, 0, 2*M_PI));
    }

    // draw custom constant parameter lines
    for(auto line : constantLines) {
        pen = QPen(line.getColor(), pref.Graphs.lineWidth);
        pen.setCosmetic(true);
        p.setPen(pen);
        for(auto arc : line.getArcs()) {
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
            if (limitToSpan && (trace->getDataType() == Trace::DataType::Frequency) && (last.x < sweep_fmin || now.x > sweep_fmax)) {
                continue;
            }
            if(isnan(now.y.real())) {
                break;
            }
            if (limitToEdge && (abs(last.y) > edgeReflection || abs(now.y) > edgeReflection)) {
                // outside of visible area
                continue;
            }
            // scale to size of smith diagram
            auto p1 = dataToPixel(last);
            auto p2 = dataToPixel(now);
            // draw line
            p.drawLine(p1, p2);
        }
        if(trace->size() > 0) {
            // only draw markers if the trace has at least one point
            auto markers = t.first->getMarkers();
            for(auto m : markers) {
//                if (m->isTimeDomain()) {
//                    continue;
//                }
                if (limitToSpan && (m->getPosition() < sweep_fmin || m->getPosition() > sweep_fmax)) {
                    continue;
                }
                if(m->getPosition() < trace->minX() || m->getPosition() > trace->maxX()) {
                    // marker not in trace range
                    continue;
                }
                auto coords = m->getData();
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
        // TODO adjust coords due to shifted restore
        p.setOpacity(0.5);
        p.setBrush(Qt::white);
        p.setPen(Qt::white);
        p.drawEllipse(-smithCoordMax, -smithCoordMax, 2*smithCoordMax, 2*smithCoordMax);
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
    Q_UNUSED(t)
    Q_UNUSED(position);
    if(supported(t)) {
        enableTrace(t, true);
    }
}

QString TraceSmithChart::mouseText(QPoint pos)
{
    auto data = pixelToData(pos);
    if(abs(data) <= edgeReflection) {
        data = Preferences::getInstance().Acquisition.refImp * (1.0 + data) / (1.0 - data);
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

void TraceSmithChart::updateContextMenu()
{
    contextmenu->clear();
    auto setup = new QAction("Setup...", contextmenu);
    connect(setup, &QAction::triggered, this, &TraceSmithChart::axisSetupDialog);
    contextmenu->addAction(setup);

    contextmenu->addSeparator();
    auto image = new QAction("Save image...", contextmenu);
    contextmenu->addAction(image);
    connect(image, &QAction::triggered, [=]() {
        auto filename = QFileDialog::getSaveFileName(nullptr, "Save plot image", "", "PNG image files (*.png)", nullptr, QFileDialog::DontUseNativeDialog);
        if(filename.isEmpty()) {
            // aborted selection
            return;
        }
        if(filename.endsWith(".png")) {
            filename.chop(4);
        }
        filename += ".png";
        grab().save(filename);
    });

    auto createMarker = contextmenu->addAction("Add marker here");
    bool activeTraces = false;
    for(auto t : traces) {
        if(t.second) {
            activeTraces = true;
            break;
        }
    }
    if(!activeTraces) {
        createMarker->setEnabled(false);
    }
    connect(createMarker, &QAction::triggered, [=](){
        createMarkerAtPosition(contextmenuClickpoint);
    });

    contextmenu->addSection("Traces");
    // Populate context menu
    for(auto t : traces) {
        if(!supported(t.first)) {
            continue;
        }
        auto action = new QAction(t.first->name(), contextmenu);
        action->setCheckable(true);
        if(t.second) {
            action->setChecked(true);
        }
        connect(action, &QAction::toggled, [=](bool active) {
            enableTrace(t.first, active);
        });
        contextmenu->addAction(action);
    }

    contextmenu->addSeparator();
    auto close = new QAction("Close", contextmenu);
    contextmenu->addAction(close);
    connect(close, &QAction::triggered, [=]() {
        markedForDeletion = true;
    });
}

bool TraceSmithChart::supported(Trace *t)
{
    return dropSupported(t);
}

bool TraceSmithChart::dropSupported(Trace *t)
{
    if(!t->isReflection()) {
        return false;
    }
    switch(t->outputType()) {
    case Trace::DataType::Frequency:
    case Trace::DataType::Power:
        return true;
    default:
        return false;
    }
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

std::vector<SmithChartArc> SmithChartConstantLine::getArcs()
{
    double Z0 = Preferences::getInstance().Acquisition.refImp;
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
