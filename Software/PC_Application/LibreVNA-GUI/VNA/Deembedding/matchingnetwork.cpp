#include "matchingnetwork.h"

#include "ui_matchingnetworkdialog.h"
#include "unit.h"
#include "CustomWidgets/informationbox.h"
#include "appwindow.h"

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
{
    dropPending = false;
    dragComponent = nullptr;
    dropComponent = nullptr;
    addNetwork = true;
    port = 1;
}

std::set<int> MatchingNetwork::getAffectedPorts()
{
    return {port};
}

void MatchingNetwork::transformDatapoint(VirtualDevice::VNAMeasurement &p)
{
    auto S = p.toSparam(1, 2);
    auto measurement = ABCDparam(S, p.Z0);
    if(matching.count(p.frequency) == 0) {
        // this point is not calculated yet
        MatchingPoint m;
        // start with identiy matrix
        m.p = ABCDparam(1.0,0.0,0.0,1.0);
        for(auto c : network) {
            m.p = m.p * c->parameters(p.frequency);
        }
        if(!addNetwork) {
            // need to remove the effect of the network, invert matrix
            m.p = m.p.inverse();
        }
        matching[p.frequency] = m;
    }
    // at this point the map contains the matching network effect
    auto m = matching[p.frequency];
    VirtualDevice::VNAMeasurement uncorrected = p;
    // correct reflection measurement (in case no two-port measurement is complete
    QString name = "S"+QString::number(port)+QString::number(port);
    if(uncorrected.measurements.count(name) > 0) {
        auto S = Sparam(uncorrected.measurements[name], 0.0, 0.0, 1.0);
        auto corrected = Sparam(m.p * ABCDparam(S, p.Z0), p.Z0);
        p.measurements[name] = corrected.m11;
    }
    // handle the rest of the measurements
    for(int i=1;i<=VirtualDevice::getInfo(VirtualDevice::getConnected()).ports;i++) {
        for(int j=i+1;j<=VirtualDevice::getInfo(VirtualDevice::getConnected()).ports;j++) {
            if(i == port) {
                auto S = uncorrected.toSparam(i, j);
                auto corrected = Sparam(m.p * ABCDparam(S, p.Z0), p.Z0);
                p.fromSparam(corrected, i, j);
            } else if(j == port) {
                auto S = uncorrected.toSparam(i, j);
                auto corrected = Sparam(ABCDparam(S, p.Z0) * m.p, p.Z0);
                p.fromSparam(corrected, i, j);
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

    ui->port->setValue(port);
    ui->port->setMaximum(VirtualDevice::getInfo(VirtualDevice::getConnected()).ports);

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
    connect(ui->buttonBox, &QDialogButtonBox::accepted, dialog, &QDialog::accept);
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
    network.clear();
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
    // remove from list when the component deletes itself
    connect(c, &MatchingComponent::deleted, [=](){
         network.erase(remove(network.begin(), network.end(), c), network.end());
    });
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
            network.erase(remove(network.begin(), network.end(), dragComponent), network.end());
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
                updateInsertIndicator(dragEvent->pos().x());
                return true;
            }
        } else if(event->type() == QEvent::DragMove) {
            auto dragEvent = static_cast<QDragMoveEvent*>(event);
            updateInsertIndicator(dragEvent->pos().x());
            return true;
        } else if(event->type() == QEvent::Drop) {
            auto dragEvent = static_cast<QDropEvent*>(event);
            delete insertIndicator;
            addComponentAtPosition(dragEvent->pos().x(), dropComponent);
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

MatchingComponent::MatchingComponent(Type type)
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
        connect(eValue, &SIUnitEdit::valueChanged, this, &MatchingComponent::valueChanged);
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
        connect(eValue, &SIUnitEdit::valueChanged, this, &MatchingComponent::valueChanged);
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
            return ABCDparam(S, 50.0);
        }
    default:
        return ABCDparam(1.0, 0.0, 0.0, 1.0);
    }
}

void MatchingComponent::MatchingComponent::setValue(double v)
{
    if(eValue) {
        eValue->setValue(v);
    }
}

MatchingComponent *MatchingComponent::createFromName(QString name)
{
    for(unsigned int i=0;i<(int) Type::Last;i++) {
        if(name == typeToName((Type) i)) {
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
    if(type == Type::DefinedThrough) {
        // select new touchstone file
        auto filename = QFileDialog::getOpenFileName(nullptr, "Open measurement file", "", "Touchstone files (*.s2p)", nullptr, QFileDialog::DontUseNativeDialog);
        if (!filename.isEmpty()) {
            try {
                *touchstone = Touchstone::fromFile(filename.toStdString());
            } catch(const std::exception& e) {
                InformationBox::ShowError("Failed to load file", QString("Attempt to load file ended with error: \"") + e.what()+"\"");
            }
            updateTouchstoneLabel();
        }
    }
}

void MatchingComponent::updateTouchstoneLabel()
{
    if(!touchstone || !touchstoneLabel) {
        return;
    }
    if(touchstone->points() == 0) {
        touchstoneLabel->setText("No data. Double-click to select touchstone file");
    } else {
        QString text = QString::number(touchstone->points()) + " points from "+Unit::ToString(touchstone->minFreq(), "Hz", " kMG", 4)
                + " to "+Unit::ToString(touchstone->maxFreq(), "Hz", " kMG", 4);
        touchstoneLabel->setText(text);
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
