#include "marker.h"
#include <QPainter>
#include "CustomWidgets/siunitedit.h"
#include <QHBoxLayout>
#include <QLabel>
#include <QSpinBox>
#include <QDebug>
#include "markermodel.h"
#include "unit.h"
#include <QMenu>
#include <QActionGroup>
#include <QApplication>
#include "preferences.h"
#include "markergroup.h"
#include "Util/util.h"

using namespace std;

Marker::Marker(MarkerModel *model, int number, Marker *parent, QString descr)
    : editingFrequency(false),
      model(model),
      parentTrace(nullptr),
      position(1000000000),
      number(number),
      data(0),
      type(Type::Manual),
      description(descr),
      contextmenu(nullptr),
      delta(nullptr),
      parent(parent),
      cutoffAmplitude(-3.0),
      peakThreshold(-40.0),
      offset(10000),
      formatTable(Format::dBAngle),
      group(nullptr)
{
    connect(this, &Marker::traceChanged, this, &Marker::updateContextmenu);
    connect(this, &Marker::typeChanged, this, &Marker::updateContextmenu);
    updateContextmenu();
}

Marker::~Marker()
{
    if(parentTrace) {
        parentTrace->removeMarker(this);
    }
    deleteHelperMarkers();
    emit deleted(this);
}

void Marker::assignTrace(Trace *t)
{
    bool firstAssignment = false;
    if(parentTrace) {
        // remove connection from previous parent trace
        parentTrace->removeMarker(this);
        disconnect(parentTrace, &Trace::deleted, this, nullptr);
    } else {
        firstAssignment = true;
    }
    parentTrace = t;
    if(!getSupportedTypes().count(type)) {
        // new trace does not support the current type
        setType(Type::Manual);
    } else {
        // helper markers might change depending on the trace, update type even when type not changed
        setType(type);
    }

    connect(parentTrace, &Trace::deleted, this, &Marker::parentTraceDeleted);
    connect(parentTrace, &Trace::dataChanged, this, &Marker::traceDataChanged);
    connect(parentTrace, &Trace::colorChanged, this, &Marker::updateSymbol);
    connect(parentTrace, &Trace::typeChanged, this, &Marker::traceTypeChanged);
    connect(parentTrace, &Trace::typeChanged, this, &Marker::checkDeltaMarker);
    constrainPosition();
    updateSymbol();
    parentTrace->addMarker(this);
    for(auto m : helperMarkers) {
        m->assignTrace(t);
    }

    if(firstAssignment) {
        // Marker was just created and this is the first assignment to a trace.
        // Use display format on graph from preferences
        auto p = Preferences::getInstance();
        if(p.Graphs.markerBehavior.showDataOnGraphs) {
            if(p.Graphs.markerBehavior.showAllData) {
                for(auto f : applicableFormats()) {
                    formatGraph.insert(f);
                }
            } else {
                formatGraph.insert(applicableFormats().front());
            }
        }
    }
    constrainFormat();
    update();
    emit traceChanged(this);
}

Trace *Marker::trace()
{
    return parentTrace;
}

QString Marker::formatToString(Marker::Format f)
{
    switch(f) {
    case Format::dB: return "dB";
    case Format::dBAngle: return "dB + angle";
    case Format::RealImag: return "real + imag";
    case Format::Impedance: return "Impedance";
    case Format::VSWR: return "VSWR";
    case Format::SeriesR: return "Resistance";
    case Format::Capacitance: return "Capacitance";
    case Format::Inductance: return "Inductance";
    case Format::QualityFactor: return "Quality Factor";
    case Format::TOI: return "Third order intercept";
    case Format::AvgTone: return "Average Tone Level";
    case Format::AvgModulationProduct: return "Average Modulation Product Level";
    case Format::Noise: return "Noise level";
    case Format::PhaseNoise: return "Phase noise";
    case Format::Cutoff: return "Cutoff frequency";
    case Format::CenterBandwidth: return "Center + Bandwidth";
    case Format::InsertionLoss: return "Insertion loss";
    case Format::P1dB: return "P1dB";
    case Format::Last: return "";
    }
    return "";
}

Marker::Format Marker::formatFromString(QString s)
{
    for(int i=0;i<(int) Format::Last;i++) {
        if(s.compare(formatToString((Format) i)) == 0) {
            return (Format) i;
        }
    }
    return Format::Last;
}

std::vector<Marker::Format> Marker::formats()
{
    std::vector<Format> ret;
    for(int i=0;i<(int) Format::Last;i++) {
        ret.push_back((Format) i);
    }
    return ret;
}

std::vector<Marker::Format> Marker::applicableFormats()
{
    std::vector<Format> ret;
    switch(getDomain()) {
    case Trace::DataType::Time:
        switch(type) {
        case Type::Manual:
        case Type::Delta:
            ret.push_back(Format::dB);
            ret.push_back(Format::RealImag);
            if(parentTrace) {
                // check if step response data is available
                auto step = parentTrace->sample(parentTrace->index(position), true).y.real();
                if(!isnan(step)) {
                    ret.push_back(Format::Impedance);
                }
            }
            break;
        default:
            break;
        }
        break;
    case Trace::DataType::Frequency:
        switch(type) {
        case Type::Manual:
        case Type::Delta:
        case Type::Maximum:
        case Type::Minimum:
        case Type::PeakTable:
            ret.push_back(Format::dB);
            ret.push_back(Format::dBAngle);
            ret.push_back(Format::RealImag);
            if(parentTrace) {
                if(parentTrace->isReflection()) {
                    ret.push_back(Format::Impedance);
                    ret.push_back(Format::VSWR);
                    ret.push_back(Format::SeriesR);
                    ret.push_back(Format::Capacitance);
                    ret.push_back(Format::Inductance);
                    ret.push_back(Format::QualityFactor);
                }
                if(!isnan(parentTrace->getNoise(parentTrace->minX()))) {
                    ret.push_back(Format::Noise);
                }
            }
            break;
        case Type::Bandpass:
            ret.push_back(Format::CenterBandwidth);
            ret.push_back(Format::InsertionLoss);
            break;
        case Type::Lowpass:
        case Type::Highpass:
            ret.push_back(Format::Cutoff);
            ret.push_back(Format::InsertionLoss);
            break;
        case Type::PhaseNoise:
            ret.push_back(Format::PhaseNoise);
            ret.push_back(Format::dB);
            break;
        case Type::TOI:
            ret.push_back(Format::TOI);
            ret.push_back(Format::AvgTone);
            ret.push_back(Format::AvgModulationProduct);
            break;
        default:
            break;
        }
        break;
    case Trace::DataType::Power:
        switch(type) {
        case Type::P1dB:
            ret.push_back(Format::P1dB);
            [[fallthrough]];
        case Type::Manual:
        case Type::Delta:
        case Type::Maximum:
        case Type::Minimum:
            ret.push_back(Format::dB);
            ret.push_back(Format::dBAngle);
            ret.push_back(Format::RealImag);
            if(parentTrace) {
                if(parentTrace->isReflection()) {
                    ret.push_back(Format::Impedance);
                    ret.push_back(Format::VSWR);
                    ret.push_back(Format::SeriesR);
                    ret.push_back(Format::Capacitance);
                    ret.push_back(Format::Inductance);
                    ret.push_back(Format::QualityFactor);
                }
            }
            break;
        default:
            break;
        }
        break;
    case Trace::DataType::Invalid:
        break;
    }
    return ret;
}

QString Marker::readableData(Format f)
{
    if(!parentTrace) {
        return "";
    }
    if(position < parentTrace->minX() || position > parentTrace->maxX()) {
        return "";
    }

    if(f == Format::Last) {
        // format not explicitly specified, use setting for table
        f = formatTable;
    }

    switch(getDomain()) {
    case Trace::DataType::Time:
        if(type != Type::Delta) {
            switch(f) {
            case Format::dB:
                return Unit::ToString(Util::SparamTodB(data), "dB", " ", 4);
            case Format::RealImag:
                return Unit::ToString(data.real(), "", " ", 5) + "+"+Unit::ToString(data.imag(), "", " ", 5)+"j";
            case Format::Impedance: {
                auto step = parentTrace->sample(parentTrace->index(position), true).y.real();
                auto impedance = Util::SparamToImpedance(step).real();
                return Unit::ToString(impedance, "Ω", "m kM", 3);
            }
                break;
            default:
                return "Invalid";
            }
        } else {
            if(!delta || delta->getDomain() != Trace::DataType::Time) {
                return "Invalid";
            }
            switch(f) {
            case Format::dB:
                return "Δ:"+Unit::ToString(Util::SparamTodB(data) - Util::SparamTodB(delta->data), "dB", " ", 4);
            case Format::RealImag:
                return "Δ:"+Unit::ToString(data.real() - delta->data.real(), "", " ", 5) + "+"+Unit::ToString(data.imag() - delta->data.real(), "", " ", 5)+"j";
            case Format::Impedance: {
                auto step = parentTrace->sample(parentTrace->index(position), true).y.real();
                auto stepDelta = delta->parentTrace->sample(delta->parentTrace->index(delta->position), true).y.real();
                auto impedance = Util::SparamToImpedance(step).real();
                auto impedanceDelta = Util::SparamToImpedance(stepDelta).real();
                return "Δ:"+Unit::ToString(impedance - impedanceDelta, "Ω", "m kM", 3);
            }
                break;
            default:
                return "Invalid";
            }
        }
        break;
    case Trace::DataType::Frequency:
    case Trace::DataType::Power:
        switch(type) {
        case Type::PeakTable:
            return "Found " + QString::number(helperMarkers.size()) + " peaks";
        case Type::Delta: {
            if(!delta) {
                return "Invalid delta marker";
            }
            switch(f) {
            case Format::dB: return "Δ:"+Unit::ToString(Util::SparamTodB(data) - Util::SparamTodB(delta->data), "dB", " ", 4);
            case Format::dBAngle: {
                QString ret = "Δ:"+Unit::ToString(Util::SparamTodB(data) - Util::SparamTodB(delta->data), "dB", " ", 4) + "/";
                auto phase = arg(data)*180/M_PI;
                auto deltaphase = arg(delta->data)*180/M_PI;
                auto phasediff = phase - deltaphase;
                if(phasediff >= 2*M_PI) {
                    phasediff -= 2*M_PI;
                } else if(phasediff <= -2*M_PI) {
                    phasediff += 2*M_PI;
                }
                ret += Unit::ToString(phasediff, "°", " ", 4);
                return ret;
            }
            case Format::RealImag: return "Δ:"+Unit::ToString(data.real() - delta->data.real(), "", " ", 5) + "+"+Unit::ToString(data.imag() - delta->data.imag(), "", " ", 5)+"j";
            case Format::Impedance: {
                auto impedance = Util::SparamToImpedance(data);
                auto delta_impedance = Util::SparamToImpedance(delta->data);
                return "Δ:"+Unit::ToString(impedance.real() - delta_impedance.real(), "Ω", "m k", 5) + "+"+Unit::ToString(impedance.imag() - delta_impedance.imag(), "Ω", "m k", 5)+"j";
            }
            case Format::SeriesR: return "Δ:"+Unit::ToString(Util::SparamToResistance(data) - Util::SparamToResistance(delta->data), "Ω", "m kM", 4);
            case Format::Capacitance: return "Δ:"+Unit::ToString(Util::SparamToCapacitance(data, position) - Util::SparamToCapacitance(delta->data, delta->position), "F", "pnum ", 4);
            case Format::Inductance: return "Δ:"+Unit::ToString(Util::SparamToInductance(data, position) - Util::SparamToInductance(delta->data, delta->position), "H", "pnum ", 4);
            case Format::QualityFactor: return "ΔQ:" + Unit::ToString(Util::SparamToQualityFactor(data) - Util::SparamToQualityFactor(delta->data), "", " ", 3);
            case Format::Noise: return "Δ:"+Unit::ToString(parentTrace->getNoise(position) - delta->parentTrace->getNoise(delta->position), "dbm/Hz", " ", 3);
            default: return "Invalid";
            }
        }
            break;
        default:
            switch(f) {
            case Format::dB: return Unit::ToString(Util::SparamTodB(data), "dB", " ", 4);
            case Format::dBAngle: return Unit::ToString(Util::SparamTodB(data), "dB", " ", 4) + "/"+Unit::ToString(arg(data)*180/M_PI, "°", " ", 4);
            case Format::RealImag: return Unit::ToString(data.real(), "", " ", 5) + "+"+Unit::ToString(data.imag(), "", " ", 5)+"j";
            case Format::VSWR:
                if(abs(data) < 1.0) {
                    return "VSWR: "+Unit::ToString(Util::SparamToVSWR(data), ":1", " ", 5);
                } else {
                    return "VSWR: NaN";
                }
                break;
            case Format::SeriesR: return Unit::ToString(Util::SparamToResistance(data), "Ω", "m kM", 4);
            case Format::Capacitance: return Unit::ToString(Util::SparamToCapacitance(data, position), "F", "pnum ", 4);
            case Format::Inductance: return Unit::ToString(Util::SparamToInductance(data, position), "H", "pnum ", 4);
            case Format::QualityFactor: return "Q:" + Unit::ToString(Util::SparamToQualityFactor(data), "", " ", 3);
            case Format::Noise: return Unit::ToString(parentTrace->getNoise(position), "dbm/Hz", " ", 3);
            case Format::TOI: {
                auto avgFundamental = (helperMarkers[0]->toDecibel() + helperMarkers[1]->toDecibel()) / 2;
                auto avgDistortion = (helperMarkers[2]->toDecibel() + helperMarkers[3]->toDecibel()) / 2;
                auto TOI = (3 * avgFundamental - avgDistortion) / 2;
                return "TOI: "+Unit::ToString(TOI, "dbm", " ", 3);
            }
                break;
            case Format::AvgTone: {
                auto avgFundamental = (helperMarkers[0]->toDecibel() + helperMarkers[1]->toDecibel()) / 2;
                return "Avg. Tone: " + Unit::ToString(avgFundamental, "dbm", " ", 3);
            }
                break;
            case Format::AvgModulationProduct: {
                auto avgDistortion = (helperMarkers[2]->toDecibel() + helperMarkers[3]->toDecibel()) / 2;
                return "Distortion: " + Unit::ToString(avgDistortion, "dbm", " ", 3);
            }
                break;
            case Format::Cutoff:
                if(parentTrace->isReflection()) {
                    return "Calculation not possible with reflection measurement";
                } else {
                    return "Cutoff:" + Unit::ToString(helperMarkers[0]->position, "Hz", " kMG", 4);
                }
            case Format::InsertionLoss:
                if(parentTrace->isReflection()) {
                    return "Calculation not possible with reflection measurement";
                } else {
                    return "Ins. Loss:"+Unit::ToString(Util::SparamTodB(data), "dB", " ", 3);
                }
            case Format::PhaseNoise: {
                auto carrier = toDecibel();
                auto phasenoise = parentTrace->getNoise(helperMarkers[0]->position) - carrier;
                return Unit::ToString(phasenoise, "dbc/Hz", " ", 3)  +"@" + Unit::ToString(offset, "Hz", " kM", 4) + " offset";
            }
                break;
            case Format::Impedance: {
                auto impedance = Util::SparamToImpedance(data);
                return Unit::ToString(impedance.real(), "Ω", "m k", 5) + "+"+Unit::ToString(impedance.imag(), "Ω", "m k", 5)+"j";
            }
            case Format::CenterBandwidth:
                if(parentTrace->isReflection()) {
                    return "Calculation not possible with reflection measurement";
                } else {
                    auto insertionLoss = toDecibel();
                    auto cutoffL = helperMarkers[0]->toDecibel();
                    auto cutoffH = helperMarkers[1]->toDecibel();
                    auto bandwidth = helperMarkers[1]->position - helperMarkers[0]->position;
                    auto center = helperMarkers[2]->position;
                    QString ret = "fc: ";
                    if(cutoffL > insertionLoss + cutoffAmplitude || cutoffH > insertionLoss + cutoffAmplitude) {
                        // the trace never dipped below the specified cutoffAmplitude, center and exact bandwidth unknown
                        ret += "?, BW: >";
                    } else {
                        ret += Unit::ToString(center, "Hz", " kMG", 5)+ ", BW: ";
                    }
                    ret += Unit::ToString(bandwidth, "Hz", " kMG", 4);
                    ret += ", Ins.Loss: >=" + QString::number(-insertionLoss, 'g', 4) + "db";
                    return ret;
                }
                break;
            case Format::P1dB:
                if(position == parentTrace->maxX()) {
                    // compression point higher than measurable with current power setting
                    return "Input P1dB:>"+Unit::ToString(position, "dBm", " ", 4);
                } else {
                    return "Input P1dB:"+Unit::ToString(position, "dBm", " ", 4);
                }
            case Format::Last:
                return "Invalid";
            }
        }
        break;
    case Trace::DataType::Invalid:
        break;
    }
    return "Invalid";
}

QString Marker::readablePosition()
{
    auto pos = position;
    QString ret;
    if(type == Type::Delta && delta) {
        pos -= delta->position;
        ret += "Δ:";
    }
    switch(getDomain()) {
    case Trace::DataType::Time:
        ret += Unit::ToString(pos, "s", "pnum ", 6);
        break;
    case Trace::DataType::Frequency:
        ret += Unit::ToString(pos, "Hz", " kMG", 6);
        break;
    case Trace::DataType::Power:
        ret += Unit::ToString(pos, "dBm", " ", 4);
        break;
    case Trace::DataType::Invalid:
        ret += "Invalid";
    }
    return ret;
}

QString Marker::readableSettings()
{
    switch(getDomain()) {
    case Trace::DataType::Time:
        switch(type) {
        case Type::Manual:
        case Type::Delta:
            return Unit::ToString(position, "s", "fpnum ", 4) + "/" + Unit::ToString(parentTrace->timeToDistance(position), "m", "um k", 4);
        default:
            return "Unhandled case";
        }
        break;
    case Trace::DataType::Frequency:
        switch(type) {
        case Type::Manual:
        case Type::Maximum:
        case Type::Minimum:
        case Type::Delta:
            return Unit::ToString(position, "Hz", " kMG", 6);
        case Type::Lowpass:
        case Type::Highpass:
        case Type::Bandpass:
            return Unit::ToString(cutoffAmplitude, "db", " ", 3);
        case Type::PeakTable:
            return Unit::ToString(peakThreshold, "db", " ", 3);
        case Type::TOI:
            return "none";
        case Type::PhaseNoise:
            return Unit::ToString(offset, "Hz", " kM", 4);
        default:
            break;
        }
        break;
    case Trace::DataType::Power:
        switch(type) {
        case Type::Manual:
        case Type::Maximum:
        case Type::Minimum:
        case Type::Delta:
            return Unit::ToString(position, "dBm", " ", 4);
        case Type::P1dB:
            return "none";
        default:
            break;
        }
        break;
    case Trace::DataType::Invalid:
        break;
    }
    return "Unhandled case";
}

QString Marker::tooltipSettings()
{
    switch(getDomain()) {
    case Trace::DataType::Time:
        switch(type) {
        case Type::Manual:
        case Type::Delta:
            return "Time/Distance";
        default:
            break;
        }
        break;
    case Trace::DataType::Frequency:
        switch(type) {
        case Type::Manual:
        case Type::Maximum:
        case Type::Minimum:
        case Type::Delta:
            return "Marker frequency";
        case Type::Lowpass:
        case Type::Highpass:
        case Type::Bandpass:
            return "Cutoff amplitude (relativ to peak)";
        case Type::PeakTable:
            return "Peak threshold";
        case Type::PhaseNoise:
            return "Frequency offset";
        default:
            break;
        }
        break;
    case Trace::DataType::Power:
        switch(type) {
        case Type::Manual:
        case Type::Maximum:
        case Type::Minimum:
        case Type::Delta:
            return "Input power position";
        default:
            return QString();
        }
        break;
    case Trace::DataType::Invalid:
        break;
    }
    return QString();
}

QString Marker::readableType()
{
    if(parent) {
        return description;
    } else {
        return typeToString(type);
    }
}

void Marker::setPosition(double pos)
{
    position = pos;
    constrainPosition();
    emit positionChanged(position);
}

void Marker::parentTraceDeleted(Trace *t)
{
    if(t == parentTrace) {
        delete this;
    }
}

void Marker::traceDataChanged()
{
    complex<double> newdata;
    if(!parentTrace || parentTrace->numSamples() == 0) {
        // no data, invalidate
        newdata = numeric_limits<complex<double>>::quiet_NaN();
    } else {
        if(position < parentTrace->minX() || position > parentTrace->maxX()) {
            // this normally should not happen because the position is constrained to the trace X range.
            // However, when loading a setup, the trace might have been just created and essentially empty
            newdata = numeric_limits<complex<double>>::quiet_NaN();
        } else {
            // some data of the parent trace changed, check if marker data also changed
            newdata = parentTrace->sample(parentTrace->index(position)).y;
        }
    }
    if (newdata != data) {
        data = newdata;
        update();
        emit rawDataChanged();
    }
}

void Marker::updateSymbol()
{
    if(isVisible()) {
        constexpr int width = 15, height = 15;
        symbol = QPixmap(width, height);
        symbol.fill(Qt::transparent);
        QPainter p(&symbol);
        p.setRenderHint(QPainter::Antialiasing);
        QPointF points[] = {QPointF(0,0),QPointF(width,0),QPointF(width/2,height)};
        auto traceColor = parentTrace->color();
        p.setPen(traceColor);
        p.setBrush(traceColor);
        p.drawConvexPolygon(points, 3);
        auto brightness = traceColor.redF() * 0.299 + traceColor.greenF() * 0.587 + traceColor.blueF() * 0.114;
        p.setPen((brightness > 0.6) ? Qt::black : Qt::white);
        p.drawText(QRectF(0,0,width, height*2.0/3.0), Qt::AlignCenter, QString::number(number) + suffix);
    } else {
        symbol = QPixmap(1,1);
    }
    emit symbolChanged(this);
}

void Marker::checkDeltaMarker()
{
    if(type != Type::Delta) {
        // not a delta marker, nothing to do
        return;
    }
    // Check if type of delta marker is still okay
    if(!delta || delta->getDomain() != getDomain()) {
        // not the same domain anymore, adjust delta
        assignDeltaMarker(bestDeltaCandidate());
    }
}

void Marker::deltaDeleted()
{
    // the delta marker of this marker has been deleted, find new match
    delta = nullptr;
    qDebug() << "assigned delta deleted";
    assignDeltaMarker(bestDeltaCandidate());
    update();
}

void Marker::updateContextmenu()
{
    if(parent) {
        // do nothing, using contextmenu from parent anyway
        return;
    }
    // check if the contextmenu or one of its submenus is currently open
    auto *activeWidget = QApplication::activePopupWidget();
    while (activeWidget) {
        if(activeWidget == &contextmenu) {
            // contextmenu currently open, do not update
            qDebug() << "Contextmenu open, skipping update";
            return;
        }
        activeWidget = activeWidget->parentWidget();
    }

    contextmenu.clear();
    contextmenu.addSection("Marker");

    auto typemenu = contextmenu.addMenu("Type");
    auto typegroup = new QActionGroup(&contextmenu);
    for(auto t : getSupportedTypes()) {
        auto setTypeAction = new QAction(typeToString(t));
        setTypeAction->setCheckable(true);
        if(t == type) {
            setTypeAction->setChecked(true);
        }
        connect(setTypeAction, &QAction::triggered, [=](){
            setType(t);
        });
        typegroup->addAction(setTypeAction);
        typemenu->addAction(setTypeAction);
    }

    auto table = contextmenu.addMenu("Data Format in Table");
    auto tablegroup = new QActionGroup(&contextmenu);
    for(auto f : applicableFormats()) {
        auto setFormatAction = new QAction(formatToString(f));
        setFormatAction->setCheckable(true);
        if(f == formatTable) {
            setFormatAction->setChecked(true);
        }
        connect(setFormatAction, &QAction::triggered, [=](){
            setTableFormat(f);
        });
        tablegroup->addAction(setFormatAction);
        table->addAction(setFormatAction);
    }

    auto graph = contextmenu.addMenu("Show on Graph");
    for(auto f : applicableFormats()) {
        auto setFormatAction = new QAction(formatToString(f));
        setFormatAction->setCheckable(true);
        if(formatGraph.count(f)) {
            setFormatAction->setChecked(true);
        }
        connect(setFormatAction, &QAction::triggered, [=](bool checked){
            if(checked) {
                formatGraph.insert(f);
            } else {
                formatGraph.erase(f);
            }
            emit dataFormatChanged(this);
        });
        graph->addAction(setFormatAction);
    }

    std::vector<MarkerGroup*> applicableGroups;
    for(auto g : model->getGroups()) {
        if(g->applicable(this)) {
            applicableGroups.push_back(g);
        }
    }

    contextmenu.addSeparator();

    bool needsSeparator = false;
    if((applicableGroups.size() > 0 && group == nullptr) || applicableGroups.size() > 1) {
        // there are other groups available than the one the marker might already be assigned to
        auto addGroupMenu = new QMenu("Add to linked group");
        auto groupGroup = new QActionGroup(addGroupMenu);
        for(auto g : model->getGroups()) {
            auto addGroupAction = new QAction(QString::number(g->getNumber()));
            groupGroup->addAction(addGroupAction);
            addGroupAction->setCheckable(true);
            if(g == group) {
                // already assigned to this group
                addGroupAction->setChecked(true);
            }
            connect(addGroupAction, &QAction::triggered, [=](bool checked){
                if(checked) {
                    g->add(this);
                }
            });
            addGroupMenu->addAction(addGroupAction);
        }
        contextmenu.addMenu(addGroupMenu);
        needsSeparator = true;
    }
    if(group != nullptr) {
        // "remove from group" available
        auto removeGroup = new QAction("Remove from linked group");
        connect(removeGroup, &QAction::triggered, [=](){
            group->remove(this);
        });
        contextmenu.addAction(removeGroup);
        needsSeparator = true;
    }
    if(needsSeparator) {
        contextmenu.addSeparator();
    }


    auto deleteAction = new QAction("Delete");
    connect(deleteAction, &QAction::triggered, this, &Marker::deleteLater);
    contextmenu.addAction(deleteAction);
}

void Marker::traceTypeChanged()
{
    // domain may have changed
    emit domainChanged(this);
}

std::set<Marker::Type> Marker::getSupportedTypes()
{
    set<Marker::Type> supported;
    if(parentTrace) {
        switch(getDomain()) {
        case Trace::DataType::Time:
            // only basic markers in time domain
            supported.insert(Type::Manual);
            supported.insert(Type::Delta);
            break;
        case Trace::DataType::Frequency:
            // all traces support some basic markers
            supported.insert(Type::Manual);
            supported.insert(Type::Maximum);
            supported.insert(Type::Minimum);
            supported.insert(Type::Delta);
            supported.insert(Type::PeakTable);
            if(!parentTrace->isReflection()) {
                supported.insert(Type::Lowpass);
                supported.insert(Type::Highpass);
                supported.insert(Type::Bandpass);
            }
            if(parentTrace->isLive()) {
                switch(parentTrace->liveParameter()) {
                case Trace::LiveParameter::S11:
                case Trace::LiveParameter::S12:
                case Trace::LiveParameter::S21:
                case Trace::LiveParameter::S22:
                    // no special marker types for VNA yet
                    break;
                case Trace::LiveParameter::Port1:
                case Trace::LiveParameter::Port2:
                    // special SA marker types
                    supported.insert(Type::TOI);
                    supported.insert(Type::PhaseNoise);
                    break;
                default: break;
                }
            }
            break;
        case Trace::DataType::Power:
            supported.insert(Type::Manual);
            supported.insert(Type::Maximum);
            supported.insert(Type::Minimum);
            supported.insert(Type::Delta);
            supported.insert(Type::P1dB);
            break;
        case Trace::DataType::Invalid:
            break;
        }
    }
    return supported;
}

void Marker::constrainPosition()
{
    if(parentTrace) {
        if(parentTrace->size() > 0)  {
            if(position > parentTrace->maxX()) {
                position = parentTrace->maxX();
            } else if(position < parentTrace->minX()) {
                position = parentTrace->minX();
            }
        }
        traceDataChanged();
    }
}

void Marker::constrainFormat()
{
    auto vec = applicableFormats();
    // check format
    if(std::find(vec.begin(), vec.end(), formatTable) == vec.end()) {
        setTableFormat(vec.front());
    }
    std::set<Format> toErase;
    for(auto f : formatGraph) {
        if(std::find(vec.begin(), vec.end(), f) == vec.end()) {
            toErase.insert(f);
        }
    }
    for(auto f : toErase) {
        formatGraph.erase(f);
    }
}

Marker *Marker::bestDeltaCandidate()
{
    Marker *match = nullptr;
    // invalid delta marker assigned, attempt to find a matching marker
    for(int pass = 0;pass < 3;pass++) {
        for(auto m : model->getMarkers()) {
            if(m->getDomain() != getDomain()) {
                // markers are not on the same domain
                continue;
            }
            if(pass == 0 && m->parentTrace != parentTrace) {
                // ignore markers on different traces in first pass
                continue;
            }
            if(pass <= 1 && m == this) {
                // ignore itself on second pass
                continue;
            }
            match = m;
            break;
        }
        if(match) {
            break;
        }
    }
    return match;
}

void Marker::assignDeltaMarker(Marker *m)
{
    if(type != Type::Delta) {
        // ignore
        return;
    }
    if(delta) {
        disconnect(delta, &Marker::dataChanged, this, &Marker::update);
    }
    delta = m;
    if(delta && delta != this) {
        // this marker has to be updated when the delta marker changes
        connect(delta, &Marker::rawDataChanged, this, &Marker::update);
        connect(delta, &Marker::domainChanged, this, &Marker::checkDeltaMarker);
        connect(delta, &Marker::deleted, this, &Marker::deltaDeleted);
    }
    emit assignedDeltaChanged(this);
}

void Marker::deleteHelperMarkers()
{
    if(helperMarkers.size() > 0) {
        emit beginRemoveHelperMarkers(this);
        for(auto m : helperMarkers) {
            delete m;
        }
        helperMarkers.clear();
        emit endRemoveHelperMarkers(this);
    }
}

void Marker::setType(Marker::Type t)
{
    // remove any potential helper markers
    deleteHelperMarkers();
    type = t;
    using helper_descr = struct {
        QString suffix;
        QString description;
        Type type;
    };
    vector<helper_descr> required_helpers;
    switch(type) {
    case Type::Delta:
        assignDeltaMarker(bestDeltaCandidate());
        break;
    case Type::Lowpass:
    case Type::Highpass:
        if(!parentTrace->isReflection()) {
            required_helpers = {{"c", "cutoff", Type::Manual}};
        }
        break;
    case Type::Bandpass:
        if(!parentTrace->isReflection()) {
            required_helpers = {{"l", "lower cutoff", Type::Manual}, {"h", "higher cutoff", Type::Manual} ,{"c", "center", Type::Manual}};
        }
        break;
    case Type::TOI:
        required_helpers = {{"p", "first peak", Type::Manual}, {"p", "second peak", Type::Manual}, {"l", "left intermodulation", Type::Manual}, {"r", "right intermodulation", Type::Manual}};
        break;
    case Type::PhaseNoise:
        required_helpers = {{"o", "Offset", Type::Manual}};
        break;
    default:
        break;
    }
    // create helper markers
    for(auto h : required_helpers) {
        auto helper = new Marker(model, number, this, h.description);
        helper->suffix = h.suffix;
        helper->assignTrace(parentTrace);
        helper->setType(h.type);
        helperMarkers.push_back(helper);
    }
    constrainFormat();

    updateSymbol();
    emit typeChanged(this);
    update();
}

double Marker::toDecibel()
{
    return Util::SparamTodB(data);
}

bool Marker::isVisible()
{
    switch(type) {
    case Type::Manual:
    case Type::Delta:
    case Type::Maximum:
    case Type::Minimum:
    case Type::PhaseNoise:
    case Type::P1dB:
        return true;
    default:
        return false;
    }
}

void Marker::setTableFormat(Marker::Format f)
{
    if(formatTable == f) {
        // already correct format, nothing to do
        return;
    }

    auto vec = applicableFormats();
    if(std::find(vec.begin(), vec.end(), f) == vec.end()) {
        // can't use requested format for this type of marker
        qWarning() << "Requested marker format" << formatToString(f) << "(not applicable for type" << typeToString(type) <<")";
        return;
    }

    formatTable = f;
    emit dataFormatChanged(this);
}

MarkerGroup *Marker::getGroup() const
{
    return group;
}

void Marker::setGroup(MarkerGroup *value)
{
    group = value;
    updateContextmenu();
}

std::set<Marker::Format> Marker::getGraphDisplayFormats() const
{
    return formatGraph;
}

Marker::Type Marker::getType() const
{
    return type;
}

QString Marker::getSuffix() const
{
    return suffix;
}

nlohmann::json Marker::toJSON()
{
    nlohmann::json j;
    j["trace"] = parentTrace->toHash();
    j["type"] = typeToString(type).toStdString();
    j["number"] = number;
    j["position"] = position;
    if(group) {
        j["group"] = group->getNumber();
    }
    switch(type) {
    case Type::Delta:
        j["delta_marker"] = delta->toHash();
        break;
    case Type::PeakTable:
        j["peak_threshold"] = peakThreshold;
        break;
    case Type::Lowpass:
    case Type::Highpass:
    case Type::Bandpass:
        j["cutoff"] = cutoffAmplitude;
        break;
    case Type::PhaseNoise:
        j["offset"] = offset;
        break;
    default:
        // other types have no settings
        break;
    }
    j["formatTable"] = formatToString(formatTable).toStdString();
    nlohmann::json jformatGraph;
    for(auto f : formatGraph) {
        jformatGraph.push_back(formatToString(f).toStdString());
    }
    j["formatGraph"] = jformatGraph;
    return j;
}

void Marker::fromJSON(nlohmann::json j)
{
    if(!j.contains("trace")) {
        throw runtime_error("Marker has no trace assigned");
    }
    number = j.value("number", 1);
    position = j.value("position", 0.0);

    unsigned int hash = j["trace"];
    // find correct trace
    bool found = false;
    for(auto t : model->getModel().getTraces()) {
        if(t->toHash() == hash) {
            found = true;
            assignTrace(t);
            break;
        }
    }
    if(!found) {
        throw runtime_error("Unable to find trace with hash " + to_string(hash));
    }
    auto typeString = QString::fromStdString(j.value("type", "Manual"));
    for(unsigned int i=0;i<(int) Type::Last;i++) {
        if(typeToString((Type) i) == typeString) {
            setType((Type) i);
            break;
        }
    }
    switch(type) {
    case Type::Delta:
        // can't assign delta marker here, because it might not have been created (if it was below this marker in the table).
        // Instead it will be correctly assigned in TraceMarkerModel::fromJSON()
        break;
    case Type::PeakTable:
        peakThreshold = j.value("peak_threshold", -40);
        break;
    case Type::Lowpass:
    case Type::Highpass:
    case Type::Bandpass:
        cutoffAmplitude = j.value("cutoff", -3.0);
        break;
    case Type::PhaseNoise:
        j.value("offset", 10000);
        break;
    default:
        // other types have no settings
        break;
    }
    formatTable = formatFromString(QString::fromStdString(j.value("formatTable", formatToString(Format::dBAngle).toStdString())));
    if(formatTable == Format::Last) {
        // invalid string, use default
        formatTable = Format::dBAngle;
    }
    formatGraph.clear();
    for(std::string s : j["formatGraph"]) {
        auto f = formatFromString(QString::fromStdString(s));
        if(f != Format::Last) {
            formatGraph.insert(f);
        }
    }
    updateContextmenu();
    update();

    if(j.contains("group")) {
        unsigned int number = j.value("group", 1);
        model->addToGroupCreateIfNotExisting(this, number);
    }
}

unsigned int Marker::toHash()
{
    // taking the easy way: create the json string and hash it (already contains all necessary information)
    // This is slower than it could be, but this function is only used when loading setups, so this isn't a big problem
    std::string json_string = toJSON().dump();
    return hash<std::string>{}(json_string);
}

const std::vector<Marker *> &Marker::getHelperMarkers() const
{
    return helperMarkers;
}

Marker *Marker::helperMarker(unsigned int i)
{
    if(i < helperMarkers.size()) {
        return helperMarkers[i];
    } else {
        return nullptr;
    }
}

Marker *Marker::getParent() const
{
    return parent;
}

void Marker::setNumber(int value)
{
    number = value;
    updateSymbol();
    for(auto h : helperMarkers) {
        h->setNumber(number);
    }
}

QWidget *Marker::getTypeEditor(QAbstractItemDelegate *delegate)
{
    auto c = new QComboBox;
    for(auto t : getSupportedTypes()) {
        c->addItem(typeToString(t));
        if(type == t) {
            // select this item
            c->setCurrentIndex(c->count() - 1);
        }
    }
    if(type == Type::Delta) {
        // add additional spinbox to choose corresponding delta marker
        auto w = new QWidget;
        auto layout = new QHBoxLayout;
        layout->addWidget(c);
        c->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
        layout->setContentsMargins(0,0,0,0);
        layout->setMargin(0);
        layout->setSpacing(0);
        layout->addWidget(new QLabel("to"));
        auto spinbox = new QSpinBox;
        if(delta) {
            spinbox->setValue(delta->number);
        }
        connect(spinbox, qOverload<int>(&QSpinBox::valueChanged), [=](int newval){
           bool found = false;
           for(auto m : model->getMarkers()) {
                if(m->number == newval) {
                    assignDeltaMarker(m);
                    found = true;
                    break;
                }
           }
           if(!found) {
               assignDeltaMarker(nullptr);
           }
           update();
        });
        spinbox->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
        layout->addWidget(spinbox);
        w->setLayout(layout);
        c->setObjectName("Type");
        if(delegate){
            connect(c, qOverload<int>(&QComboBox::currentIndexChanged), [=](int) {
                emit delegate->commitData(w);
            });
        }
        return w;
    } else {
        // no delta marker, simply return the combobox
        connect(c, qOverload<int>(&QComboBox::currentIndexChanged), [=](int) {
            emit delegate->commitData(c);
        });
        return c;
    }
}

void Marker::updateTypeFromEditor(QWidget *w)
{
    QComboBox *c;
    if(type == Type::Delta) {
        c = w->findChild<QComboBox*>("Type");
    } else {
        c = (QComboBox*) w;
    }
    for(auto t : getSupportedTypes()) {
        if(c->currentText() == typeToString(t)) {
            if(type != t) {
                setType(t);
            }
        }
    }
    update();
}

SIUnitEdit *Marker::getSettingsEditor()
{
    SIUnitEdit *ret = nullptr;
    switch(getDomain()) {
    case Trace::DataType::Time:
        switch(type) {
        case Type::Manual:
        case Type::Delta:
            ret = new SIUnitEdit("", "fpnum k", 6);
            ret->setValue(position);
            break;
        default:
            return nullptr;
        }
        break;
    case Trace::DataType::Frequency:
        switch(type) {
        case Type::Manual:
        case Type::Maximum:
        case Type::Minimum:
        case Type::Delta:
            ret = new SIUnitEdit("Hz", " kMG", 6);
            ret->setValue(position);
            break;
        case Type::PhaseNoise:
            ret = new SIUnitEdit("Hz", " kMG", 6);
            ret->setValue(offset);
            break;
        case Type::Lowpass:
        case Type::Highpass:
        case Type::Bandpass:        // initialize with "dB"
            ret = new SIUnitEdit("db", " ", 3);
            ret->setValue(cutoffAmplitude);
            break;
        case Type::PeakTable:
            ret = new SIUnitEdit("db", " ", 3);
            ret->setValue(peakThreshold);
            break;
        default:
            return nullptr;
        }
        break;
    case Trace::DataType::Power:
        switch(type) {
        case Type::Manual:
        case Type::Maximum:
        case Type::Minimum:
        case Type::Delta:
            ret = new SIUnitEdit("dBm", " ", 4);
            ret->setValue(position);
            break;
        default:
            return nullptr;
        }
        break;
    case Trace::DataType::Invalid:
        return nullptr;
    }
    return ret;
}

void Marker::adjustSettings(double value)
{
    switch(getDomain()) {
    case Trace::DataType::Time:
        switch(type) {
        case Type::Manual:
        case Type::Delta: {
            // check if entered position is time or distance
            if(value > parentTrace->sample(parentTrace->size() - 1).x) {
                // entered a distance, convert to time
                setPosition(parentTrace->distanceToTime(value));
            } else {
                // entered a time, can set directly
                setPosition(value);
            }
        }
        default:
            break;
        }
        break;
    case Trace::DataType::Frequency:
        switch(type) {
        case Type::Manual:
        case Type::Maximum:
        case Type::Minimum:
        case Type::Delta:
            setPosition(value);
            break;
        case Type::Lowpass:
        case Type::Highpass:
        case Type::Bandpass:
            if(value > 0.0) {
                value = -value;
            }
            cutoffAmplitude = value;
            break;
        case Type::PeakTable:
            peakThreshold = value;
            break;
        case Type::PhaseNoise:
            offset = value;
            break;
        default:
            break;
        }
        break;
    case Trace::DataType::Power:
        switch(type) {
        case Type::Manual:
        case Type::Maximum:
        case Type::Minimum:
        case Type::Delta:
            setPosition(value);
            break;
        default:
            break;
        }
        break;
    case Trace::DataType::Invalid:
        break;
    }
    update();
}

QMenu *Marker::getContextMenu() {
    if(parent) {
        return parent->getContextMenu();
    } else {
        return &contextmenu;
    }
}

void Marker::update()
{
    if(!parentTrace->size()) {
        // empty trace, nothing to do
        return;
    }
    switch(type) {
    case Type::Manual:
    case Type::Delta:
        // nothing to do
        break;
    case Type::Maximum:
        setPosition(parentTrace->findExtremum(true));
        break;
    case Type::Minimum:
        setPosition(parentTrace->findExtremum(false));
        break;
    case Type::PeakTable: {
        deleteHelperMarkers();
        auto peaks = parentTrace->findPeakFrequencies(100, peakThreshold);
        char suffix = 'a';
        for(auto p : peaks) {
            auto helper = new Marker(model, number, this);
            helper->suffix = suffix;
            helper->assignTrace(parentTrace);
            helper->setPosition(p);
            helper->formatTable = formatTable;
            helper->formatGraph = formatGraph;
            helper->updateContextmenu();
            suffix++;
            helperMarkers.push_back(helper);
        }
    }
        break;
    case Type::Lowpass:
    case Type::Highpass:
        if(parentTrace->isReflection()) {
            // lowpass/highpass calculation only works with transmission measurement
            break;
        } else {
            // find the maximum
            auto peakFreq = parentTrace->findExtremum(true);
            // this marker shows the insertion loss
            setPosition(peakFreq);
            // find the cutoff frequency
            auto index = parentTrace->index(peakFreq);
            auto peakAmplitude = Util::SparamTodB(parentTrace->sample(index).y);
            auto cutoff = peakAmplitude + cutoffAmplitude;
            int inc = type == Type::Lowpass ? 1 : -1;
            while(index >= 0 && index < (int) parentTrace->size()) {
                auto amplitude = Util::SparamTodB(parentTrace->sample(index).y);
                if(amplitude <= cutoff) {
                    break;
                }
                index += inc;
            }
            if(index < 0) {
                index = 0;
            } else if(index >= (int) parentTrace->size()) {
                index = parentTrace->size() - 1;
            }
            // set position of cutoff marker
            helperMarkers[0]->setPosition(parentTrace->sample(index).x);
        }
        break;
    case Type::Bandpass:
        if(parentTrace->isReflection()) {
            // lowpass/highpass calculation only works with transmission measurement
            break;
        } else {
            // find the maximum
            auto peakFreq = parentTrace->findExtremum(true);
            // this marker shows the insertion loss
            setPosition(peakFreq);
            // find the cutoff frequencies
            auto index = parentTrace->index(peakFreq);
            auto peakAmplitude = Util::SparamTodB(parentTrace->sample(index).y);
            auto cutoff = peakAmplitude + cutoffAmplitude;

            auto low_index = index;
            while(low_index >= 0) {
                auto amplitude = Util::SparamTodB(parentTrace->sample(low_index).y);
                if(amplitude <= cutoff) {
                    break;
                }
                low_index--;
            }
            if(low_index < 0) {
                low_index = 0;
            }
            // set position of cutoff marker
            helperMarkers[0]->setPosition(parentTrace->sample(low_index).x);

            auto high_index = index;
            while(high_index < (int) parentTrace->size()) {
                auto amplitude = Util::SparamTodB(parentTrace->sample(high_index).y);
                if(amplitude <= cutoff) {
                    break;
                }
                high_index++;
            }
            if(high_index >= (int) parentTrace->size()) {
                high_index = parentTrace->size() - 1;
            }
            // set position of cutoff marker
            helperMarkers[1]->setPosition(parentTrace->sample(high_index).x);
            // set center marker inbetween cutoff markers
            helperMarkers[2]->setPosition((helperMarkers[0]->position + helperMarkers[1]->position) / 2);
        }
        break;
    case Type::TOI: {
        auto peaks = parentTrace->findPeakFrequencies(2);
        if(peaks.size() != 2) {
            // error finding peaks, do nothing
            break;
        }
        // assign marker frequenies:
        // this marker is the left peak, first helper the right peak.
        // 2nd and 3rd helpers are left and right TOI peaks
        helperMarkers[0]->setPosition(peaks[0]);
        helperMarkers[1]->setPosition(peaks[1]);
        auto freqDiff = peaks[1] - peaks[0];
        helperMarkers[2]->setPosition(peaks[0] - freqDiff);
        helperMarkers[3]->setPosition(peaks[1] + freqDiff);
    }
        break;
    case Type::PhaseNoise:
        setPosition(parentTrace->findExtremum(true));
        helperMarkers[0]->setPosition(position + offset);
        break;
    case Type::P1dB: {
        // find maximum
        auto maxpos = parentTrace->findExtremum(true);
        // starting at the maximum point, traverse trace data towards higher power levels until amplitude dropped by 1dB
        auto maxindex = parentTrace->index(maxpos);
        auto maxpower = abs(parentTrace->sample(maxindex).y);
        double p1db = parentTrace->maxX();
        for(unsigned int i = maxindex; i < parentTrace->size(); i++) {
            auto sample = parentTrace->sample(i);
            if(Util::SparamTodB(maxpower) - Util::SparamTodB(sample.y) >= 1.0) {
                p1db = sample.x;
                break;
            }
        }
        setPosition(p1db);
    }
        break;
    case Type::Last:
        break;
    }
    emit dataChanged(this);
}

Trace *Marker::getTrace() const
{
    return parentTrace;
}

int Marker::getNumber() const
{
    return number;
}

std::complex<double> Marker::getData() const
{
    return data;
}

bool Marker::isMovable()
{
    if(parent) {
        // helper traces are never movable by the user
        return false;
    }
    switch(type) {
    case Type::Manual:
    case Type::Delta:
        return true;
    default:
        return false;
    }
}

QPixmap &Marker::getSymbol()
{
    return symbol;
}

double Marker::getPosition() const
{
    return position;
}

Trace::DataType Marker::getDomain()
{
    if(parentTrace) {
        return parentTrace->outputType();
    }
    return Trace::DataType::Invalid;
}

