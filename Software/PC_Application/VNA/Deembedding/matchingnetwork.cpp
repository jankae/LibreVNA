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
}

void MatchingNetwork::transformDatapoint(VNAData &p)
{
    auto S = p.S;
    auto measurement = ABCDparam(S, p.reference_impedance);
    if(matching.count(p.frequency) == 0) {
        // this point is not calculated yet
        MatchingPoint m;
        // start with identiy matrix
        m.p1 = ABCDparam(1.0,0.0,0.0,1.0);
        for(auto c : p1Network) {
            m.p1 = m.p1 * c->parameters(p.frequency);
        }
        // same for network at port 2
        m.p2 = ABCDparam(1.0,0.0,0.0,1.0);
        for(auto c : p2Network) {
            m.p2 = m.p2 * c->parameters(p.frequency);
        }
        if(!addNetwork) {
            // need to remove the effect of the networks, invert matrices
            m.p1 = m.p1.inverse();
            m.p2 = m.p2.inverse();
        }
        matching[p.frequency] = m;
    }
    // at this point the map contains the matching network effect
    auto m = matching[p.frequency];
    auto corrected = m.p1 * measurement * m.p2;
    p.S = Sparam(corrected, p.reference_impedance);
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
    layout->setContentsMargins(0,0,0,0);
    layout->setSpacing(0);
    layout->addStretch(1);
    auto p1 = new QWidget();
    p1->setMinimumSize(portWidth, 151);
    p1->setMaximumSize(portWidth, 151);
    p1->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    p1->setStyleSheet("image: url(:/icons/port1.png);");
    auto DUT = new QWidget();
    DUT->setMinimumSize(DUTWidth, 151);
    DUT->setMaximumSize(DUTWidth, 151);
    DUT->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    DUT->setStyleSheet("image: url(:/icons/DUT.png);");
    auto p2 = new QWidget();
    p2->setMinimumSize(portWidth, 151);
    p2->setMaximumSize(portWidth, 151);
    p2->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    p2->setStyleSheet("image: url(:/icons/port2.png);");
    layout->addWidget(p1);
    for(auto w : p1Network) {
        layout->addWidget(w);
        connect(w, &MatchingComponent::MatchingComponent::valueChanged, [=](){
           matching.clear();
        });
    }
    layout->addWidget(DUT);
    for(auto w : p2Network) {
        layout->addWidget(w);
        connect(w, &MatchingComponent::MatchingComponent::valueChanged, [=](){
           matching.clear();
        });
    }
    layout->addWidget(p2);

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
    connect(ui->buttonBox, &QDialogButtonBox::accepted, dialog, &QDialog::accept);
}

nlohmann::json MatchingNetwork::toJSON()
{
    nlohmann::json j;
    nlohmann::json jn1, jn2;
    for(auto c : p1Network) {
        nlohmann::json jc;
        jc["component"] = c->getName().toStdString();
        jc["params"] = c->toJSON();
        jn1.push_back(jc);
    }
    for(auto c : p2Network) {
        nlohmann::json jc;
        jc["component"] = c->getName().toStdString();
        jc["params"] = c->toJSON();
        jn2.push_back(jc);
    }
    j["port1"] = jn1;
    j["port2"] = jn2;
    j["addNetwork"] = addNetwork;
    return j;
}

void MatchingNetwork::fromJSON(nlohmann::json j)
{
    p1Network.clear();
    p2Network.clear();
    if(j.contains("port1")) {
        for(auto jc : j["port1"]) {
            if(!jc.contains("component")) {
                continue;
            }
            auto c = MatchingComponent::createFromName(QString::fromStdString(jc["component"]));
            if(!c) {
                continue;
            }
            c->fromJSON(jc["params"]);
            p1Network.push_back(c);
        }
    }
    if(j.contains("port2")) {
        for(auto jc : j["port2"]) {
            if(!jc.contains("component")) {
                continue;
            }
            auto c = MatchingComponent::createFromName(QString::fromStdString(jc["component"]));
            if(!c) {
                continue;
            }
            c->fromJSON(jc["params"]);
            p2Network.push_back(c);
        }
    }
    addNetwork = j.value("addNetwork", true);
    matching.clear();
}

MatchingComponent *MatchingNetwork::componentAtPosition(int pos)
{
    pos -= graph->layout()->itemAt(0)->geometry().width();
    pos -= portWidth;
    if(pos > 0 && pos <= (int) p1Network.size() * componentWidth) {
        // position is in port 1 network
        return p1Network[pos / componentWidth];
    } else if(pos > (int) p1Network.size() * componentWidth + DUTWidth) {
        pos -= (int) p1Network.size() * componentWidth + DUTWidth;
        if(pos <= (int) p2Network.size() * componentWidth) {
                // position is in port 2 network
                return p2Network[pos / componentWidth];
        }
    }
    return nullptr;
}

unsigned int MatchingNetwork::findInsertPosition(int xcoord)
{
    xcoord -= graph->layout()->itemAt(0)->geometry().width();
    xcoord -= portWidth;
    if(xcoord <= (int) p1Network.size() * componentWidth + DUTWidth/2) {
        // added in port 1 network
        int index = (xcoord + componentWidth / 2) / componentWidth;
        if(index < 0) {
            index = 0;
        } else if(index > (int) p1Network.size()) {
            index = p1Network.size();
        }
        // add 2 (first two widgets are always the stretch and port 1 widget)
        return index + 2;
    } else {
        // added in port 2 network
        xcoord -= (int) p1Network.size() * componentWidth + DUTWidth;
        int index = (xcoord + componentWidth / 2) / componentWidth;
        if(index < 0) {
            index = 0;
        } else if(index > (int) p2Network.size()) {
            index = p2Network.size();
        }
        // add 3 (same two widgets as in port 1 + DUT) and the size of the port 1 network
        return index + 3 + p1Network.size();
    }
}

void MatchingNetwork::addComponentAtPosition(int pos, MatchingComponent *c)
{
    auto index = findInsertPosition(pos);
    QHBoxLayout *l = static_cast<QHBoxLayout*>(graph->layout());
    l->insertWidget(index, c);
    c->show();

    // add component to correct matching network
    index -= 2; // first two widgets are fixed
    if(index <= p1Network.size()) {
        addComponent(true, index, c);
    } else {
        index -= 1 + p1Network.size();
        addComponent(false, index, c);
    }

    // network changed, need to recalculate matching
    matching.clear();
    connect(c, &MatchingComponent::valueChanged, [=](){
       matching.clear();
    });
}

void MatchingNetwork::addComponent(bool port1, int index, MatchingComponent *c)
{
    if(port1) {
        p1Network.insert(p1Network.begin() + index, c);
        // remove from list when the component deletes itself
        connect(c, &MatchingComponent::deleted, [=](){
             p1Network.erase(remove(p1Network.begin(), p1Network.end(), c), p1Network.end());
        });
    } else {
        // same procedure for port 2 network
        p2Network.insert(p2Network.begin() + index, c);
        // remove from list when the component deletes itself
        connect(c, &MatchingComponent::deleted, [=](){
             p2Network.erase(remove(p2Network.begin(), p2Network.end(), c), p2Network.end());
        });
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
            p1Network.erase(remove(p1Network.begin(), p1Network.end(), dragComponent), p1Network.end());
            p2Network.erase(remove(p2Network.begin(), p2Network.end(), dragComponent), p2Network.end());
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
