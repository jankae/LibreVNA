#include "matchingnetwork.h"

#include "ui_matchingnetworkdialog.h"
#include "unit.h"
#include "CustomWidgets/informationbox.h"
#include "appwindow.h"
#include "Util/util.h"

#include <QDialog>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QSpacerItem>
#include <QMouseEvent>
#include <QDrag>
#include <QMimeData>
#include <algorithm>
#include <QDebug>
#include <QFileDialog>

using namespace std;

MatchingNetwork::MatchingNetwork()
    : DeembeddingOption("MATCHing")
{
    dropPending = false;
    dragComponent = nullptr;
    dropComponent = nullptr;
    addNetwork = true;
    port = 1;

    graph = nullptr;
    insertIndicator = nullptr;

    addUnsignedIntParameter("PORT", port);
    addBoolParameter("ADD", addNetwork);
    add(new SCPICommand("CLEAR", [=](QStringList params) -> QString {
        Q_UNUSED(params);
        clearNetwork();
        return SCPI::getResultName(SCPI::Result::Empty);
    }, nullptr));
    add(new SCPICommand("NUMber", nullptr, [=](QStringList params) -> QString {
        Q_UNUSED(params);
        return QString::number(network.size());
    }));
    add(new SCPICommand("NEW", [=](QStringList params) -> QString {
        if(params.size() < 1) {
            return SCPI::getResultName(SCPI::Result::Error);
        }
        auto c = MatchingComponent::createFromName(params[0].replace("_", " "));
        if(!c) {
            return SCPI::getResultName(SCPI::Result::Error);
        }
        unsigned long long index = network.size();
        // parse index (unchanged if not provided)
        SCPI::paramToULongLong(params, 1, index);
        addComponent(index, c);
        return SCPI::getResultName(SCPI::Result::Empty);
    }, nullptr));
    add(new SCPICommand("TYPE", nullptr, [=](QStringList params) -> QString {
        unsigned long long index = 0;
        if(!SCPI::paramToULongLong(params, 0, index)) {
            return SCPI::getResultName(SCPI::Result::Error);
        }
        if(index < 1 || index > network.size()) {
            return SCPI::getResultName(SCPI::Result::Error);
        }
        return network[index-1]->getName().replace(" ", "_");
    }));
}

MatchingNetwork::~MatchingNetwork()
{
    clearNetwork();
}

std::set<unsigned int> MatchingNetwork::getAffectedPorts()
{
    return {port};
}

void MatchingNetwork::transformDatapoint(DeviceDriver::VNAMeasurement &p)
{
    if(matching.count(p.frequency) == 0) {
        // this point is not calculated yet
        MatchingPoint m;
        // start with identiy matrix
        m.forward = ABCDparam(1.0,0.0,0.0,1.0);
        m.reverse = ABCDparam(1.0,0.0,0.0,1.0);
        for(unsigned int i=0;i<network.size();i++) {
            m.forward = m.forward * network[i]->parameters(p.frequency);
            m.reverse = m.reverse * network[network.size()-i-1]->parameters(p.frequency);
        }
        if(!addNetwork) {
            // need to remove the effect of the network, invert matrix
            m.forward = m.forward.inverse();
            m.reverse = m.reverse.inverse();
        }
        matching[p.frequency] = m;
    }
    // at this point the map contains the matching network effect
    auto m = matching[p.frequency];
    DeviceDriver::VNAMeasurement uncorrected = p;

    auto portReflectionName = "S"+QString::number(port)+QString::number(port);
    if(!uncorrected.measurements.count(portReflectionName)) {
        // the reflection measurement for the port to de-embed is not included, nothing can be done
        return;
    }
    // calculate internal reflection at the matching port
    auto portReflectionS = uncorrected.measurements[portReflectionName];
    auto matchingReflectionS = Sparam(m.forward, p.Z0).get(2,2);
    auto internalPortReflectionS = matchingReflectionS / (1.0 - matchingReflectionS * portReflectionS);

    // handle the measurements
    for(auto &meas : p.measurements) {
        QString name = meas.first;
        if(!name.startsWith("S")) {
            continue;
        }
        unsigned int i = name.mid(1,1).toUInt();
        unsigned int j = name.mid(2,1).toUInt();
        if(i == j) {
            // reflection measurement
            if(i == port) {
                // the port of the matching network itself
                auto S = Sparam(uncorrected.measurements[name], 1.0, 1.0, 0.0);
                auto corrected = Sparam(m.forward * ABCDparam(S, p.Z0), p.Z0);
                p.measurements[name] = corrected.get(1,1);
            } else {
                // another reflection measurement
                try {
                    auto S = uncorrected.toSparam().reduceTo({i, port});
                    auto corrected = Sparam(ABCDparam(S, p.Z0) * m.reverse, p.Z0);
                    p.fromSparam(corrected, {i, port});
                } catch (...) {
                    // missing measurements, nothing can be done
                }
            }
        } else {
            // through measurement
            if(i != port && j != port) {
                try {
                    // find through measurements from these two ports to and from the embedding port
                    auto toPort = uncorrected.measurements["S"+QString::number(port)+QString::number(j)];
                    auto fromPort = uncorrected.measurements["S"+QString::number(i)+QString::number(port)];
                    p.measurements[name] = p.measurements[name] + toPort * internalPortReflectionS * fromPort;
                } catch (...) {
                    // missing measurements, nothing can be done
                }
            } else {
                // Already handled by reflection measurement (toSparam uses S12/S21 as well)
                // and if the corresponding reflection measurement is not available, we can't
                // do anything anyway
            }
        }
    }
}

void MatchingNetwork::edit()
{
    auto dialog = new QDialog();
    auto ui = new Ui::MatchingNetworkDialog();
    ui->setupUi(dialog);
    connect(dialog, &QDialog::finished, [=](){
        delete ui;
    });
    dialog->setModal(true);

    graph = new QWidget();
    ui->scrollArea->setWidget(graph);
    auto layout = new QHBoxLayout();
    graph->setLayout(layout);
    graph->setAcceptDrops(true);
    graph->setObjectName("Graph");
    graph->installEventFilter(this);
    ui->lSeriesC->installEventFilter(this);
    ui->lSeriesL->installEventFilter(this);
    ui->lSeriesR->installEventFilter(this);
    ui->lParallelC->installEventFilter(this);
    ui->lParallelL->installEventFilter(this);
    ui->lParallelR->installEventFilter(this);
    ui->lDefinedThrough->installEventFilter(this);
    ui->lDefinedShunt->installEventFilter(this);

    ui->port->setValue(port);
    ui->port->setMaximum(DeviceDriver::maximumSupportedPorts);

    layout->setContentsMargins(0,0,0,0);
    layout->setSpacing(0);
    layout->addStretch(1);
    auto p1 = new QWidget();
    p1->setMinimumSize(portWidth, 151);
    p1->setMaximumSize(portWidth, 151);
    p1->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    p1->setStyleSheet("image: url(:/icons/port.png);");
    auto DUT = new QWidget();
    DUT->setMinimumSize(DUTWidth, 151);
    DUT->setMaximumSize(DUTWidth, 151);
    DUT->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    DUT->setStyleSheet("image: url(:/icons/DUT_onePort.png);");

    layout->addWidget(p1);
    for(auto w : network) {
        layout->addWidget(w);
        connect(w, &MatchingComponent::MatchingComponent::valueChanged, [=](){
           matching.clear();
        });
    }
    layout->addWidget(DUT);

    layout->addStretch(1);

    if(AppWindow::showGUI()) {
        dialog->show();
    }
    if(addNetwork) {
        ui->bAddNetwork->setChecked(true);
    } else {
        ui->bRemoveNetwork->setChecked(true);
    }
    connect(ui->bAddNetwork, &QRadioButton::toggled, [=](bool add) {
        addNetwork = add;
        // network changed, need to recalculate matching
        matching.clear();
    });
    connect(ui->port, qOverload<int>(&QSpinBox::valueChanged), [=](){
        port = ui->port->value();
    });
    connect(ui->buttonBox, &QDialogButtonBox::accepted, [=](){
        graph = nullptr;
        dialog->accept();
    });
}

nlohmann::json MatchingNetwork::toJSON()
{
    nlohmann::json j;
    nlohmann::json jn;
    for(auto c : network) {
        nlohmann::json jc;
        jc["component"] = c->getName().toStdString();
        jc["params"] = c->toJSON();
        jn.push_back(jc);
    }
    j["port"] = port;
    j["network"] = jn;
    j["addNetwork"] = addNetwork;
    return j;
}

void MatchingNetwork::fromJSON(nlohmann::json j)
{
    clearNetwork();
    port = j.value("port", 1);
    if(j.contains("network")) {
        for(auto jc : j["network"]) {
            if(!jc.contains("component")) {
                continue;
            }
            auto c = MatchingComponent::createFromName(QString::fromStdString(jc["component"]));
            if(!c) {
                continue;
            }
            c->fromJSON(jc["params"]);
            network.push_back(c);
        }
    }
    addNetwork = j.value("addNetwork", true);
    matching.clear();
}

void MatchingNetwork::clearNetwork()
{
    while(network.size() > 0) {
        auto c = network[0];
        removeComponent(c);
        delete c;
    }
}

MatchingComponent *MatchingNetwork::componentAtPosition(int pos)
{
    pos -= graph->layout()->itemAt(0)->geometry().width();
    pos -= portWidth;
    if(pos > 0 && pos <= (int) network.size() * componentWidth) {
        // position is in port 1 network
        return network[pos / componentWidth];
    }
    return nullptr;
}

unsigned int MatchingNetwork::findInsertPosition(int xcoord)
{
    xcoord -= graph->layout()->itemAt(0)->geometry().width();
    xcoord -= portWidth;
    // added in port 1 network
    int index = (xcoord + componentWidth / 2) / componentWidth;
    if(index < 0) {
        index = 0;
    } else if(index > (int) network.size()) {
        index = network.size();
    }
    // add 2 (first two widgets are always the stretch and port 1 widget)
    return index + 2;
}

void MatchingNetwork::addComponentAtPosition(int pos, MatchingComponent *c)
{
    auto index = findInsertPosition(pos);
    QHBoxLayout *l = static_cast<QHBoxLayout*>(graph->layout());
    l->insertWidget(index, c);
    c->show();

    // add component to correct matching network
    index -= 2; // first two widgets are fixed
    addComponent(index, c);

    // network changed, need to recalculate matching
    matching.clear();
    connect(c, &MatchingComponent::valueChanged, [=](){
       matching.clear();
    });
}

void MatchingNetwork::addComponent(int index, MatchingComponent *c)
{
    network.insert(network.begin() + index, c);
    updateSCPINames();
    if(graph) {
        graph->update();
    }
    matching.clear();
    // remove from list when the component deletes itself
    connect(c, &MatchingComponent::deleted, [=](){
         removeComponent(c);
    });
}

void MatchingNetwork::removeComponent(int index)
{
    network.erase(network.begin() + index);
    matching.clear();
    updateSCPINames();
    if(graph) {
        graph->update();
    }
}

void MatchingNetwork::removeComponent(MatchingComponent *c)
{
    network.erase(std::remove(network.begin(), network.end(), c), network.end());
    matching.clear();
    updateSCPINames();
    if(graph) {
        graph->update();
    }
}

void MatchingNetwork::createDragComponent(MatchingComponent *c)
{
    QDrag *drag = new QDrag(this);
    QMimeData *mimeData = new QMimeData;

    QByteArray encodedPointer;
    QDataStream stream(&encodedPointer, QIODevice::WriteOnly);
    stream << quintptr(c);

    mimeData->setData("matchingComponent/pointer", encodedPointer);
    drag->setMimeData(mimeData);

    drag->exec(Qt::MoveAction);
}

void MatchingNetwork::updateInsertIndicator(int xcoord)
{
    auto index = findInsertPosition(xcoord);
    QHBoxLayout *l = static_cast<QHBoxLayout*>(graph->layout());
    l->removeWidget(insertIndicator);
    l->insertWidget(index, insertIndicator);
}

bool MatchingNetwork::eventFilter(QObject *object, QEvent *event)
{
    if(object->objectName() == "Graph") {
        if(event->type() == QEvent::MouseButtonPress) {
            auto mouseEvent = static_cast<QMouseEvent*>(event);
            if (mouseEvent->button() == Qt::LeftButton) {
                dragComponent = componentAtPosition(mouseEvent->pos().x());
                if(dragComponent) {
                    dragStartPosition = mouseEvent->pos();
                    return true;
                }
            }
            return false;
        } else if(event->type() == QEvent::MouseMove) {
            auto mouseEvent = static_cast<QMouseEvent*>(event);
            if (!(mouseEvent->buttons() & Qt::LeftButton)) {
                return false;
            }
            if (!dragComponent) {
                return false;
            }
            if ((mouseEvent->pos() - dragStartPosition).manhattanLength()
                 < QApplication::startDragDistance()) {
                return false;
            }

            // remove and hide component while it is being dragged
            graph->layout()->removeWidget(dragComponent);
            dragComponent->hide();
            removeComponent(dragComponent);
            graph->update();

            // network changed, need to recalculate matching
            matching.clear();

            createDragComponent(dragComponent);
            return true;
        } else if(event->type() == QEvent::DragEnter) {
            auto dragEvent = static_cast<QDragEnterEvent*>(event);
            if(dragEvent->mimeData()->hasFormat("matchingComponent/pointer")) {
                dropPending = true;
                auto data = dragEvent->mimeData()->data("matchingComponent/pointer");
                QDataStream stream(&data, QIODevice::ReadOnly);
                quintptr dropPtr;
                stream >> dropPtr;
                dropComponent = (MatchingComponent*) dropPtr;
                dragEvent->acceptProposedAction();
                insertIndicator = new QWidget();
                insertIndicator->setMinimumSize(2, imageHeight);
                insertIndicator->setMaximumSize(2, imageHeight);
                insertIndicator->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
                insertIndicator->setStyleSheet("background-color:red;");
                updateInsertIndicator(dragEvent->position().toPoint().x());
                return true;
            }
        } else if(event->type() == QEvent::DragMove) {
            auto dragEvent = static_cast<QDragMoveEvent*>(event);
            updateInsertIndicator(dragEvent->position().toPoint().x());
            return true;
        } else if(event->type() == QEvent::Drop) {
            auto dragEvent = static_cast<QDropEvent*>(event);
            delete insertIndicator;
            addComponentAtPosition(dragEvent->position().toPoint().x(), dropComponent);
            return true;
        } else if(event->type() == QEvent::DragLeave) {
            dropPending = false;
            dropComponent = nullptr;
            delete insertIndicator;
        }
    } else {
        // clicked/dragged one of the components outside of the graph
        if(event->type() == QEvent::MouseButtonPress) {
            auto mouseEvent = static_cast<QMouseEvent*>(event);
            if (mouseEvent->button() == Qt::LeftButton) {
                dragStartPosition = mouseEvent->pos();
                if(object->objectName() == "lSeriesC") {
                    dragComponent = new MatchingComponent(MatchingComponent::Type::SeriesC);
                } else if(object->objectName() == "lSeriesL") {
                    dragComponent = new MatchingComponent(MatchingComponent::Type::SeriesL);
                } else if(object->objectName() == "lSeriesR") {
                    dragComponent = new MatchingComponent(MatchingComponent::Type::SeriesR);
                } else if(object->objectName() == "lParallelC") {
                    dragComponent = new MatchingComponent(MatchingComponent::Type::ParallelC);
                } else if(object->objectName() == "lParallelL") {
                    dragComponent = new MatchingComponent(MatchingComponent::Type::ParallelL);
                } else if(object->objectName() == "lParallelR") {
                    dragComponent = new MatchingComponent(MatchingComponent::Type::ParallelR);
                } else if(object->objectName() == "lDefinedThrough") {
                    dragComponent = new MatchingComponent(MatchingComponent::Type::DefinedThrough);
                } else if(object->objectName() == "lDefinedShunt") {
                    dragComponent = new MatchingComponent(MatchingComponent::Type::DefinedShunt);
                } else {
                    dragComponent = nullptr;
                }
                return true;
            }
            return false;
        } else if(event->type() == QEvent::MouseMove) {
            auto mouseEvent = static_cast<QMouseEvent*>(event);
            if (!(mouseEvent->buttons() & Qt::LeftButton)) {
                return false;
            }
            if (!dragComponent) {
                return false;
            }
            if ((mouseEvent->pos() - dragStartPosition).manhattanLength()
                 < QApplication::startDragDistance()) {
                return false;
            }

            createDragComponent(dragComponent);
            return true;
        }
    }
    return false;
}

void MatchingNetwork::updateSCPINames()
{
    // Need to remove all components from the subnode list first, otherwise
    // name changes wouldn't work due to temporarily name collisions
    for(auto &c : network) {
        remove(c);
    }
    unsigned int i=1;
    for(auto &c : network) {
        c->changeName(QString::number(i));
        add(c);
        i++;
    }
}

MatchingComponent::MatchingComponent(Type type)
    : SCPINode("COMPONENT")
{
    this->type = type;
    eValue = nullptr;
    touchstone = nullptr;
    touchstoneLabel = nullptr;
    setMinimumSize(151, 151);
    setMaximumSize(151, 151);
    setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    setFocusPolicy(Qt::FocusPolicy::ClickFocus);
    switch(type) {
    case Type::SeriesR:
    case Type::SeriesL:
    case Type::SeriesC: {
        eValue = new SIUnitEdit();
        eValue->setPrecision(4);
        eValue->setPrefixes("fpnum k");
        connect(eValue, &SIUnitEdit::valueChanged, [=](double newval) {
            value = newval;
            emit valueChanged();
        });
        auto layout = new QVBoxLayout();
        layout->addWidget(eValue);
        setLayout(layout);
    }
        break;
    case Type::ParallelR:
    case Type::ParallelL:
    case Type::ParallelC: {
        eValue = new SIUnitEdit();
        eValue->setPrecision(4);
        eValue->setPrefixes("fpnum k");
        connect(eValue, &SIUnitEdit::valueChanged, [=](double newval) {
            value = newval;
            emit valueChanged();
        });
        auto layout = new QVBoxLayout();
        layout->addWidget(eValue);
        layout->addStretch(1);
        layout->setContentsMargins(9, 5, 9, 9);
        setLayout(layout);
    }
    default:
        break;
    }
    switch(type) {
    case Type::SeriesR:
        eValue->setUnit("Ω");
        eValue->setValue(50);
        setStyleSheet("image: url(:/icons/seriesR.png);");
        break;
    case Type::SeriesL:
        eValue->setUnit("H");
        eValue->setValue(1e-9);
        setStyleSheet("image: url(:/icons/seriesL.png);");
        break;
    case Type::SeriesC:
        eValue->setUnit("F");
        eValue->setValue(1e-12);
        setStyleSheet("image: url(:/icons/seriesC.png);");
        break;
    case Type::ParallelR:
        eValue->setUnit("Ω");
        eValue->setValue(50);
        setStyleSheet("image: url(:/icons/parallelR.png);");
        break;
    case Type::ParallelL:
        eValue->setUnit("H");
        eValue->setValue(1e-9);
        setStyleSheet("image: url(:/icons/parallelL.png);");
        break;
    case Type::ParallelC:
        eValue->setUnit("F");
        eValue->setValue(1e-12);
        setStyleSheet("image: url(:/icons/parallelC.png);");
        break;
    case Type::DefinedThrough: {
        touchstone = new Touchstone(2);
        touchstoneLabel = new QLabel();
        touchstoneLabel->setWordWrap(true);
        touchstoneLabel->setAlignment(Qt::AlignCenter);
        auto layout = new QVBoxLayout();
        layout->addWidget(touchstoneLabel);
        layout->setContentsMargins(0, 0, 0, 0);
        setLayout(layout);
        setStyleSheet("image: url(:/icons/definedThrough.png);");
        updateTouchstoneLabel();
    }
        break;
    case Type::DefinedShunt: {
        touchstone = new Touchstone(2);
        touchstoneLabel = new QLabel();
        touchstoneLabel->setWordWrap(true);
        touchstoneLabel->setAlignment(Qt::AlignCenter);
        auto layout = new QVBoxLayout();
        layout->addWidget(touchstoneLabel);
        layout->setContentsMargins(0, 0, 0, 0);
        setLayout(layout);
        setStyleSheet("image: url(:/icons/definedShunt.png);");
        updateTouchstoneLabel();
    }
        break;
    default:
        break;
    }
    switch(type) {
    case Type::SeriesR:
    case Type::SeriesL:
    case Type::SeriesC:
    case Type::ParallelR:
    case Type::ParallelL:
    case Type::ParallelC:
        addDoubleParameter("VALue", value, true, true, [=](){
            eValue->setValue(value);
        });
        break;
    case Type::DefinedThrough:
    case Type::DefinedShunt:
        add(new SCPICommand("FILE", [=](QStringList params) -> QString {
            if(params.size() < 1) {
                return SCPI::getResultName(SCPI::Result::Error);
            }
            try {
                *touchstone = Touchstone::fromFile(params[0].toStdString());
                updateTouchstoneLabel();
                emit valueChanged();
                return SCPI::getResultName(SCPI::Result::Empty);
            } catch(const std::exception& e) {
                // failed to load file
                return SCPI::getResultName(SCPI::Result::Error);
            }
        }, nullptr));
        break;
    default:
        break;
    }
}

MatchingComponent::~MatchingComponent()
{
    delete eValue;
    delete touchstone;
    delete touchstoneLabel;
}

ABCDparam MatchingComponent::parameters(double freq)
{
    switch(type) {
    case Type::SeriesR:
        return ABCDparam(1.0, eValue->value(), 0.0, 1.0);
    case Type::SeriesL:
        return ABCDparam(1.0, complex<double>(0, freq * 2 * M_PI * eValue->value()), 0.0, 1.0);
    case Type::SeriesC:
        return ABCDparam(1.0, complex<double>(0, -1.0 / (freq * 2 * M_PI * eValue->value())), 0.0, 1.0);
    case Type::ParallelR:
        return ABCDparam(1.0, 0.0, 1.0/eValue->value(), 1.0);
    case Type::ParallelL:
        return ABCDparam(1.0, 0.0, 1.0/complex<double>(0, freq * 2 * M_PI * eValue->value()), 1.0);
    case Type::ParallelC:
        return ABCDparam(1.0, 0.0, 1.0/complex<double>(0, -1.0 / (freq * 2 * M_PI * eValue->value())), 1.0);
    case Type::DefinedThrough:
        if(touchstone->points() == 0 || freq < touchstone->minFreq() || freq > touchstone->maxFreq()) {
            // outside of provided frequency range, pass through unchanged
            return ABCDparam(1.0, 0.0, 0.0, 1.0);
        } else {
            auto d = touchstone->interpolate(freq);
            auto S = Sparam(d.S[0], d.S[1], d.S[2], d.S[3]);
            return ABCDparam(S, touchstone->getReferenceImpedance());
        }
    case Type::DefinedShunt:
        if(touchstone->points() == 0 || freq < touchstone->minFreq() || freq > touchstone->maxFreq()) {
            // outside of provided frequency range, pass through unchanged
            return ABCDparam(1.0, 0.0, 0.0, 1.0);
        } else {
            auto d = touchstone->interpolate(freq);
            auto Y = Yparam(Sparam(d.S[0], d.S[1], d.S[2], d.S[3]), touchstone->getReferenceImpedance());
            return ABCDparam(1.0, 0.0, Y.get(1,1), 1.0);
        }
    default:
        return ABCDparam(1.0, 0.0, 0.0, 1.0);
    }
}

void MatchingComponent::MatchingComponent::setValue(double v)
{
    value = v;
    if(eValue) {
        eValue->setValue(v);
    }
}

MatchingComponent *MatchingComponent::createFromName(QString name)
{
    for(unsigned int i=0;i<(int) Type::Last;i++) {
        if(typeToName((Type) i).compare(name, Qt::CaseInsensitive) == 0) {
            return new MatchingComponent((Type) i);
        }
    }
    // invalid name
    return nullptr;
}

QString MatchingComponent::getName()
{
    return typeToName(type);
}

nlohmann::json MatchingComponent::toJSON()
{
    nlohmann::json j;
    switch(type) {
    case Type::SeriesC:
    case Type::SeriesR:
    case Type::SeriesL:
    case Type::ParallelC:
    case Type::ParallelR:
    case Type::ParallelL:
        j["value"] = eValue->value();
        break;
    case Type::DefinedThrough:
    case Type::DefinedShunt:
        j["touchstone"] = touchstone->toJSON();
        break;
    case Type::Last:
        break;
    }
    return j;
}

void MatchingComponent::fromJSON(nlohmann::json j)
{
    switch(type) {
    case Type::SeriesC:
    case Type::SeriesR:
    case Type::SeriesL:
    case Type::ParallelC:
    case Type::ParallelR:
    case Type::ParallelL:
        eValue->setValue(j.value("value", 1e-12));
        break;
    case Type::DefinedThrough:
    case Type::DefinedShunt:
        touchstone->fromJSON(j["touchstone"]);
        updateTouchstoneLabel();
        break;
    case Type::Last:
        break;
    }
}

void MatchingComponent::mouseDoubleClickEvent(QMouseEvent *e)
{
    Q_UNUSED(e);
    if(type == Type::DefinedThrough || type == Type::DefinedShunt) {
        // select new touchstone file
        auto filename = QFileDialog::getOpenFileName(nullptr, "Open measurement file", "", "Touchstone files (*.s2p)", nullptr, Preferences::QFileDialogOptions());
        if (!filename.isEmpty()) {
            try {
                *touchstone = Touchstone::fromFile(filename.toStdString());
            } catch(const std::exception& e) {
                InformationBox::ShowError("Failed to load file", QString("Attempt to load file ended with error: \"") + e.what()+"\"");
            }
            updateTouchstoneLabel();
            emit valueChanged();
        }
    }
}

void MatchingComponent::updateTouchstoneLabel()
{
    if(!touchstone || !touchstoneLabel) {
        return;
    }
    QFont font = touchstoneLabel->font();
    font.setPointSize(10);
    touchstoneLabel->setFont(font);
    if(touchstone->points() == 0) {
        touchstoneLabel->setText("No data. Double-click to select touchstone file");
    } else {
        QString text = QString::number(touchstone->points()) + " points from "+Unit::ToString(touchstone->minFreq(), "Hz", " kMG", 4)
                + " to "+Unit::ToString(touchstone->maxFreq(), "Hz", " kMG", 4);
        touchstoneLabel->setText(text);
    }
    if(type == Type::DefinedThrough) {
        touchstoneLabel->setAlignment(Qt::AlignCenter);
    } else if(type == Type::DefinedShunt) {
        touchstoneLabel->setAlignment(Qt::AlignTop | Qt::AlignHCenter);
    }
}

QString MatchingComponent::typeToName(MatchingComponent::Type type)
{
    switch(type) {
    case Type::SeriesR: return "SeriesR";
    case Type::SeriesL: return "SeriesL";
    case Type::SeriesC: return "SeriesC";
    case Type::ParallelR: return "ParallelR";
    case Type::ParallelL: return "ParallelL";
    case Type::ParallelC: return "ParallelC";
    case Type::DefinedThrough: return "Touchstone Through";
    case Type::DefinedShunt: return "Touchstone Shunt";
    default: return "";
    }
}

void MatchingComponent::MatchingComponent::keyPressEvent(QKeyEvent *event)
{
    if(event->key() == Qt::Key_Delete) {
        emit deleted(this);
        delete this;
    }
}

void MatchingComponent::MatchingComponent::focusInEvent(QFocusEvent *event)
{
    Q_UNUSED(event);
    oldStylesheet = styleSheet();
    setStyleSheet(styleSheet().append("\nborder: 1px solid red;"));
}

void MatchingComponent::MatchingComponent::focusOutEvent(QFocusEvent *event)
{
    Q_UNUSED(event);
    setStyleSheet(oldStylesheet);
}
