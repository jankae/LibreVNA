#include "compounddeviceeditdialog.h"
#include "ui_compounddeviceeditdialog.h"

#include "compounddriver.h"

#include <QPushButton>
#include <QDrag>
#include <QMimeData>
#include <QMouseEvent>
#include <QStandardItemModel>

using namespace std;

CompoundDeviceEditDialog::CompoundDeviceEditDialog(CompoundDevice *cdev, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::CompoundDeviceEditDialog),
    dropPending(false),
    dragFrame(nullptr),
    dropFrame(nullptr)
{
    ldev = *cdev;
    ui->setupUi(this);

    connect(ui->name, &QLineEdit::editingFinished, [=](){
       ldev.name = ui->name->text();
       checkIfOkay();
    });
    for(int i=0;i<(int)LibreVNADriver::Synchronization::Last;i++) {
        ui->sync->addItem(CompoundDevice::SyncToString((LibreVNADriver::Synchronization) i));
        switch((LibreVNADriver::Synchronization) i) {
        case LibreVNADriver::Synchronization::Disabled:
        case LibreVNADriver::Synchronization::Reserved: {
            // Disable for now
            auto *model = qobject_cast<QStandardItemModel *>(ui->sync->model());
            Q_ASSERT(model != nullptr);
            bool disabled = true;
            auto *item = model->item(i);
            item->setFlags(disabled ? item->flags() & ~Qt::ItemIsEnabled
                                  : item->flags() | Qt::ItemIsEnabled);
        }
            break;
        default:
            break;
        }
    }
    connect(ui->sync, &QComboBox::currentTextChanged, [=](){
       ldev.sync = CompoundDevice::SyncFromString(ui->sync->currentText());
       updateDeviceFrames();
    });

    connect(ui->buttonBox->button(QDialogButtonBox::Ok), &QPushButton::clicked, [=](){
        *cdev = ldev;
        accept();
    });
    connect(ui->buttonBox->button(QDialogButtonBox::Cancel), &QPushButton::clicked, this, &QDialog::reject);

    connect(ui->buttonBox->button(QDialogButtonBox::Save), &QPushButton::clicked, [=](){
        ldev.saveToFileDialog("Save compound device", "Compound device file (*.cdev)", ".cdev");
    });
    connect(ui->buttonBox->button(QDialogButtonBox::Open), &QPushButton::clicked, [=](){
        if(ldev.openFromFileDialog("Load compound device", "Compound device file (*.cdev)")) {
            setInitialGUI();
        }
    });

    ui->status->setStyleSheet("QLabel { color : red; }");

    graph = new QWidget();
    ui->scrollArea->setWidget(graph);
    auto layout = new QHBoxLayout();
    graph->setLayout(layout);
    graph->setAcceptDrops(true);
    graph->setObjectName("Graph");
    graph->installEventFilter(this);
    ui->lLibreVNA1->installEventFilter(this);
    layout->setContentsMargins(0,0,0,0);
    layout->setSpacing(0);

    setInitialGUI();
}

CompoundDeviceEditDialog::~CompoundDeviceEditDialog()
{
    delete ui;
}

void CompoundDeviceEditDialog::checkIfOkay()
{
    ui->buttonBox->button(QDialogButtonBox::Ok)->setEnabled(false);
    ui->buttonBox->button(QDialogButtonBox::Save)->setEnabled(false);

    if(deviceFrames.size() != ldev.deviceSerials.size()) {
        // probably still populating the GUI, skip check
        return;
    }

    if(ldev.name.isEmpty()) {
        ui->status->setText("Name must not be empty");
        return;
    }
    if(ldev.deviceSerials.size() < 2) {
        ui->status->setText("At least two devices are required");
        return;
    }
    if(ldev.deviceSerials.size() > 4) {
        ui->status->setText("Only up to four devices supported");
        return;
    }
    // Check serials
    for(unsigned int i=0;i<ldev.deviceSerials.size();i++) {
        ldev.deviceSerials[i] = deviceFrames[i]->getSerial();
        if(ldev.deviceSerials[i].isEmpty()) {
            ui->status->setText("Device "+QString::number(i+1)+" has no serial number");
            return;
        }
    }
    // Check serials for duplicates
    for(unsigned int i=0;i<ldev.deviceSerials.size();i++) {
        for(unsigned int j=i+1;j<ldev.deviceSerials.size();j++) {
            if(ldev.deviceSerials[i] == ldev.deviceSerials[j]) {
                ui->status->setText("Duplicate serial number ("+ldev.deviceSerials[i]+") in devices "+QString::number(i+1)+" and "+QString::number(j+1));
                return;
            }
        }
    }
    // Check port mapping
    // Looking for duplicate and missing ports
    bool highestPortFound = false;
    unsigned int highestPort = 0;
    for(unsigned int port=0;port<2*ldev.deviceSerials.size();port++) {
        int num = 0;
        for(unsigned int i=0;i<deviceFrames.size();i++) {
            if(deviceFrames[i]->getPort1() == (int) port) {
                num++;
            }
            if(deviceFrames[i]->getPort2() == (int) port) {
                num++;
            }
        }
        if(num > 1) {
            ui->status->setText("Duplicate port "+QString::number(port+1));
            return;
        } else if(num == 0) {
            if(port == 0) {
                ui->status->setText("Port 1 must be present");
                return;
            }
            if(!highestPortFound) {
                highestPort = port;
            }
            highestPortFound = true;
        } else if(highestPortFound) {
            // port is present, but earlier port was missing
            ui->status->setText("Missing port: "+QString::number(port+1)+" is selected, but port "+QString::number(port)+" is not present.");
            return;
        }
    }
    if(!highestPortFound) {
        highestPort = 2*ldev.deviceSerials.size();
    }

    // All good, actually create the port mapping
    ldev.portMapping.clear();
    for(unsigned int port=0;port<highestPort;port++) {
        CompoundDevice::PortMapping map;
        bool found = false;
        for(unsigned int i=0;i<deviceFrames.size();i++) {
            if(deviceFrames[i]->getPort1() == (int) port) {
                found = true;
                map.device = i;
                map.port = 0;
                break;
            }
            if(deviceFrames[i]->getPort2() == (int) port) {
                found = true;
                map.device = i;
                map.port = 1;
                break;
            }
        }
        if(!found) {
            ui->status->setText("Failed to find port "+QString::number(port+1)+" (likely a bug)");
            return;
        }
        ldev.portMapping.push_back(map);
    }

    ui->status->clear();

    ui->buttonBox->button(QDialogButtonBox::Save)->setEnabled(true);
    ui->buttonBox->button(QDialogButtonBox::Ok)->setEnabled(true);
}

void CompoundDeviceEditDialog::setInitialGUI()
{
    ui->name->setText(ldev.name);
    ui->sync->setCurrentText(CompoundDevice::SyncToString(ldev.sync));

    // Removing the old frames actually modifies the state of ldev as well. Block signals to prevent this from happening
    for(auto f : deviceFrames) {
        f->blockSignals(true);
    }

    QHBoxLayout* layout = (QHBoxLayout*) graph->layout();
    // remove all widgets from the layout
    if (layout != NULL ) {
        QLayoutItem* item;
        while ((item = layout->takeAt(0)) != NULL ) {
            delete item->widget();
            delete item;
        }
    }
    deviceFrames.clear();
    layout->addStretch(1);
    for(unsigned int i=0;i<ldev.deviceSerials.size();i++) {
        auto frame = new DeviceFrame(&ldev, i);
        connect(frame, &DeviceFrame::settingChanged, this, &CompoundDeviceEditDialog::checkIfOkay);
        addFrame(i, frame);
    }
    layout->addStretch(1);

    checkIfOkay();
}

DeviceFrame *CompoundDeviceEditDialog::frameAtPosition(int pos)
{
    pos -= graph->layout()->itemAt(0)->geometry().width();
    if(pos > 0 && pos <= (int) deviceFrames.size() * frameSize) {
        return deviceFrames[pos / frameSize];
    }
    return nullptr;
}

unsigned int CompoundDeviceEditDialog::findInsertPosition(int xcoord)
{
    xcoord -= graph->layout()->itemAt(0)->geometry().width();
    // added in port 1 network
    int index = (xcoord + frameSize / 2) / frameSize;
    if(index < 0) {
        index = 0;
    } else if(index > (int) deviceFrames.size()) {
        index = deviceFrames.size();
    }
    // add 1 (first widget is always the stretch)
    return index + 1;
}

void CompoundDeviceEditDialog::addFrameAtPosition(int pos, DeviceFrame *c)
{
    auto index = findInsertPosition(pos);

    // add component to the deviceFrame vector
    index -= 1; // first widget is fixed
    if(index <= deviceFrames.size()) {
        addFrame(index, c);
    }
}

void CompoundDeviceEditDialog::addFrame(int index, DeviceFrame *c)
{
    if(deviceFrames.size() == ldev.deviceSerials.size()) {
        ldev.deviceSerials.insert(ldev.deviceSerials.begin() + index, "");
    }
    deviceFrames.insert(deviceFrames.begin() + index, c);
    deviceFrames[index]->setPosition(index);
    // remove from list when the component deletes itself
    connect(c, &DeviceFrame::deleted, [=](){
        removeDeviceFrame(c);
    });
    QHBoxLayout *l = static_cast<QHBoxLayout*>(graph->layout());
    l->insertWidget(index + 1, c);
    c->show();
    updateDeviceFrames();
}

void CompoundDeviceEditDialog::createDragFrame(DeviceFrame *c)
{
    QDrag *drag = new QDrag(this);
    QMimeData *mimeData = new QMimeData;

    QByteArray encodedPointer;
    QDataStream stream(&encodedPointer, QIODevice::WriteOnly);
    stream << quintptr(c);

    mimeData->setData("compoundDeviceFrame/pointer", encodedPointer);
    drag->setMimeData(mimeData);

    drag->exec(Qt::MoveAction);
    dragFrame = nullptr;
}

void CompoundDeviceEditDialog::updateInsertIndicator(int xcoord)
{
    auto index = findInsertPosition(xcoord);
    QHBoxLayout *l = static_cast<QHBoxLayout*>(graph->layout());
    l->removeWidget(insertIndicator);
    l->insertWidget(index, insertIndicator);
}

bool CompoundDeviceEditDialog::eventFilter(QObject *object, QEvent *event)
{
    if(object->objectName() == "Graph") {
        if(event->type() == QEvent::MouseButtonPress) {
            auto mouseEvent = static_cast<QMouseEvent*>(event);
            if (mouseEvent->button() == Qt::LeftButton) {
                dragFrame = frameAtPosition(mouseEvent->pos().x());
                if(dragFrame) {
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
            if (!dragFrame) {
                return false;
            }
            if ((mouseEvent->pos() - dragStartPosition).manhattanLength()
                 < QApplication::startDragDistance()) {
                return false;
            }

            // remove and hide component while it is being dragged
            graph->layout()->removeWidget(dragFrame);
            dragFrame->hide();
            removeDeviceFrame(dragFrame);
            graph->update();

            createDragFrame(dragFrame);
            return true;
        } else if(event->type() == QEvent::DragEnter) {
            auto dragEvent = static_cast<QDragEnterEvent*>(event);
            if(dragEvent->mimeData()->hasFormat("compoundDeviceFrame/pointer")) {
                dropPending = true;
                auto data = dragEvent->mimeData()->data("compoundDeviceFrame/pointer");
                QDataStream stream(&data, QIODevice::ReadOnly);
                quintptr dropPtr;
                stream >> dropPtr;
                dropFrame = (DeviceFrame*) dropPtr;
                dragEvent->acceptProposedAction();
                insertIndicator = new QWidget();
                insertIndicator->setMinimumSize(2, frameSize);
                insertIndicator->setMaximumSize(2, frameSize);
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
            addFrameAtPosition(dragEvent->position().toPoint().x(), dropFrame);
            return true;
        } else if(event->type() == QEvent::DragLeave) {
            dropPending = false;
            dropFrame = nullptr;
            delete insertIndicator;
        }
    } else {
        // clicked/dragged one of the components outside of the graph
        if(event->type() == QEvent::MouseButtonPress) {
            auto mouseEvent = static_cast<QMouseEvent*>(event);
            if (mouseEvent->button() == Qt::LeftButton) {
                dragStartPosition = mouseEvent->pos();
                if(object->objectName() == "lLibreVNA1") {
                    dragFrame = new DeviceFrame(&ldev, 0);
                    connect(dragFrame, &DeviceFrame::settingChanged, this, &CompoundDeviceEditDialog::checkIfOkay);
                } else {
                    dragFrame = nullptr;
                }
                return true;
            }
            return false;
        } else if(event->type() == QEvent::MouseMove) {
            auto mouseEvent = static_cast<QMouseEvent*>(event);
            if (!(mouseEvent->buttons() & Qt::LeftButton)) {
                return false;
            }
            if (!dragFrame) {
                return false;
            }
            if ((mouseEvent->pos() - dragStartPosition).manhattanLength()
                 < QApplication::startDragDistance()) {
                return false;
            }

            createDragFrame(dragFrame);
            return true;
        }
    }
    return false;
}

void CompoundDeviceEditDialog::removeDeviceFrame(DeviceFrame *dev)
{
    auto it = std::find(deviceFrames.begin(), deviceFrames.end(), dev);
    if(it == deviceFrames.end()) {
        // not found, shouldn't happen
        return;
    }
    auto pos = it - deviceFrames.begin();
    deviceFrames.erase(it);
    ldev.deviceSerials.erase(ldev.deviceSerials.begin() + pos);
    // remove all port mappings from the removed device
    bool mappingFound;
    do {
        mappingFound = false;
        for(unsigned int i=0;i<ldev.portMapping.size();i++) {
            if(ldev.portMapping[i].device == pos) {
                mappingFound = true;
                ldev.portMapping.erase(ldev.portMapping.begin() + i);
                break;
            }
        }
    } while(mappingFound);
    updateDeviceFrames();
}

void CompoundDeviceEditDialog::updateDeviceFrames()
{
    int i=0;
    for(auto df : deviceFrames) {
        df->setPosition(i++);
    }
}

DeviceFrame::DeviceFrame(CompoundDevice *dev, unsigned int position) :
    position(position),
    dev(dev)
{
    setMinimumSize(frameSize, frameSize);
    setMaximumSize(frameSize, frameSize);
    setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    setFocusPolicy(Qt::FocusPolicy::ClickFocus);

    serial = new QComboBox();
    serial->setEditable(true);
    port1 = new QComboBox();
    port2 = new QComboBox();

    connect(serial, &QComboBox::currentTextChanged, this, &DeviceFrame::settingChanged);
    connect(port1, &QComboBox::currentTextChanged, this, &DeviceFrame::settingChanged);
    connect(port2, &QComboBox::currentTextChanged, this, &DeviceFrame::settingChanged);

    auto layout = new QVBoxLayout;
    layout->addStretch(1);
    auto l1 = new QHBoxLayout;
    l1->addStretch(1);
    l1->addWidget(serial);
    l1->addStretch(1);
    layout->addLayout(l1);
    layout->addStretch(1);
    auto l2 = new QHBoxLayout;
    l2->addWidget(port1);
    l2->addStretch(1);
    l2->addWidget(port2);
    layout->addLayout(l2);
    setLayout(layout);

    update();

    // Set initial state
    if(position < dev->deviceSerials.size()) {
        serial->setCurrentText(dev->deviceSerials[position]);
        for(unsigned int i=0;i<dev->portMapping.size();i++) {
            if(dev->portMapping[i].device == position) {
                if(dev->portMapping[i].port == 0) {
                    port1->setCurrentIndex(i + 1);
                } else if(dev->portMapping[i].port == 1) {
                    port2->setCurrentIndex(i + 1);
                }
            }
        }
    }
}

DeviceFrame::~DeviceFrame()
{
    emit deleted();
}

void DeviceFrame::setPosition(int pos)
{
    position = pos;
    update();
}

void DeviceFrame::update()
{
    auto p1 = port1->currentText();
    auto p2 = port2->currentText();
    auto s = serial->currentText();
    port1->clear();
    port2->clear();
    serial->clear();

    port1->addItem("Unused");
    port2->addItem("Unused");
    for(unsigned int i=0;i<dev->deviceSerials.size();i++) {
        port1->addItem("Port "+QString::number(i*2+1));
        port2->addItem("Port "+QString::number(i*2+1));
        port1->addItem("Port "+QString::number(i*2+2));
        port2->addItem("Port "+QString::number(i*2+2));
    }
    if(port1->findText(p1) >= 0) {
        port1->setCurrentText(p1);
    } else {
        port1->setCurrentIndex(0);
    }
    if(port2->findText(p2) >= 0) {
        port2->setCurrentText(p2);
    } else {
        port2->setCurrentIndex(0);
    }

    for(auto d : CompoundDriver::getIndividualDeviceSerials()) {
        serial->addItem(d);
    }
    serial->setCurrentText(s);

    if(dev->sync == LibreVNADriver::Synchronization::GUI) {
        setStyleSheet("image: url(:/icons/compound_V1_USB.png);");
    } else if(dev->sync == LibreVNADriver::Synchronization::Reserved) {
        if(position == 0) {
            setStyleSheet("image: url(:/icons/compound_V1_Ref_Left.png);");
        } else if(position == dev->deviceSerials.size() - 1) {
            setStyleSheet("image: url(:/icons/compound_V1_Ref_Right.png);");
        } else {
            setStyleSheet("image: url(:/icons/compound_V1_Ref_Middle.png);");
        }
    }
}

QString DeviceFrame::getSerial()
{
    return serial->currentText();
}

int DeviceFrame::getPort1()
{
    return port1->currentIndex() - 1;
}

int DeviceFrame::getPort2()
{
    return port2->currentIndex() - 1;
}
