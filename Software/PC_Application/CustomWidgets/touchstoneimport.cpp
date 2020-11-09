#include "touchstoneimport.h"
#include "ui_touchstoneimport.h"
#include <QFileDialog>
#include <QDebug>
#include <QtGlobal>

using namespace std;

TouchstoneImport::TouchstoneImport(QWidget *parent, int ports) :
    QWidget(parent),
    ui(new Ui::TouchstoneImport),
    touchstone(ports),
    status(false)
{
    ui->setupUi(this);
    ui->lowerFreq->setUnit("Hz");
    ui->lowerFreq->setPrefixes(" kMG");
    ui->upperFreq->setUnit("Hz");
    ui->upperFreq->setPrefixes(" kMG");
    connect(ui->browse, &QPushButton::clicked, this, &TouchstoneImport::evaluateFile);
    ui->port1Group->setId(ui->port1_1, 0);
    ui->port1Group->setId(ui->port1_2, 1);
    ui->port1Group->setId(ui->port1_3, 2);
    ui->port1Group->setId(ui->port1_4, 3);
    ui->port2Group->setId(ui->port2_1, 0);
    ui->port2Group->setId(ui->port2_2, 1);
    ui->port2Group->setId(ui->port2_3, 2);
    ui->port2Group->setId(ui->port2_4, 3);
    // prevent selection of same port for port1 and 2
    connect(ui->port1Group, qOverload<int>(&QButtonGroup::buttonClicked), [=](int id) {
        preventCollisionWithGroup(ui->port2Group, id);
    });
    connect(ui->port2Group, qOverload<int>(&QButtonGroup::buttonClicked), [=](int id) {
        preventCollisionWithGroup(ui->port1Group, id);
    });
    setPorts(ports);
}

TouchstoneImport::~TouchstoneImport()
{
    delete ui;
}

bool TouchstoneImport::getStatus()
{
    return status;
}

Touchstone TouchstoneImport::getTouchstone()
{
    if(required_ports == 1) {
        auto t1 = touchstone;
        t1.reduceTo1Port(ui->port1Group->checkedId());
        return t1;
    } else if(required_ports == 2) {
        auto t2 = touchstone;
        t2.reduceTo2Port(ui->port1Group->checkedId(), ui->port2Group->checkedId());
        return t2;
    }

    return touchstone;
}

void TouchstoneImport::setPorts(int ports)
{
    required_ports = ports;
    ui->port1Widget->setVisible(ports >= 1);
    ui->port2Widget->setVisible(ports >= 2);
}

QString TouchstoneImport::getFilename()
{
    return ui->file->text();
}

void TouchstoneImport::selectPort(int destination, int source)
{
    switch(destination) {
    case 0:
        ui->port1Group->button(source)->setChecked(true);
        preventCollisionWithGroup(ui->port2Group, source);
        break;
    case 1:
        ui->port2Group->button(source)->setChecked(true);
        preventCollisionWithGroup(ui->port1Group, source);
        break;
    }
}

std::vector<int> TouchstoneImport::getPorts()
{
    vector<int> ret;
    if(required_ports >= 1) {
        ret.push_back(ui->port1Group->checkedId());
    }
    if(required_ports >= 2) {
        ret.push_back(ui->port2Group->checkedId());
    }
    return ret;
}

void TouchstoneImport::setFile(QString filename)
{
    ui->file->setText(filename);
    evaluateFile();
}

void TouchstoneImport::on_browse_clicked()
{
    auto filename = QFileDialog::getOpenFileName(nullptr, "Open measurement file", "", "Touchstone files (*.s1p *.s2p *.s3p *.s4p)", nullptr, QFileDialog::DontUseNativeDialog);
    if (filename.length() > 0) {
        ui->file->setText(filename);
        evaluateFile();
    }
}

void TouchstoneImport::evaluateFile()
{
    bool new_status = false;
    ui->port1_1->setEnabled(false);
    ui->port1_2->setEnabled(false);
    ui->port1_3->setEnabled(false);
    ui->port1_4->setEnabled(false);
    if (required_ports != 1) {
        ui->port2_1->setEnabled(false);
        ui->port2_2->setEnabled(false);
        ui->port2_3->setEnabled(false);
        ui->port2_4->setEnabled(false);
    }
    ui->points->setText("");
    ui->lowerFreq->setText("");
    ui->upperFreq->setText("");
    ui->status->clear();
    try {
        touchstone = Touchstone::fromFile(ui->file->text().toStdString());
        if (required_ports > 0 && touchstone.ports() < (unsigned int) required_ports) {
            throw runtime_error("Not enough ports in file");
        }
        ui->port1_1->setEnabled(touchstone.ports() >= 1);
        ui->port1_2->setEnabled(touchstone.ports() >= 2);
        ui->port1_3->setEnabled(touchstone.ports() >= 3);
        ui->port1_4->setEnabled(touchstone.ports() >= 4);
        if (required_ports != 1) {
            ui->port2_1->setEnabled(touchstone.ports() >= 1);
            ui->port2_2->setEnabled(touchstone.ports() >= 2);
            ui->port2_3->setEnabled(touchstone.ports() >= 3);
            ui->port2_4->setEnabled(touchstone.ports() >= 4);
        }
        ui->points->setText(QString::number(touchstone.points()));
        ui->lowerFreq->setValue(touchstone.minFreq());
        ui->upperFreq->setValue(touchstone.maxFreq());
        if(ui->port1Group->checkedId() == -1 || !ui->port1Group->checkedButton()->isEnabled()) {
            // no or invalid S parameter selected
            ui->port1_1->setChecked(true);
        }
        if (required_ports != 1) {
            preventCollisionWithGroup(ui->port2Group, 0);
        }
        new_status = true;
    } catch (const exception &e) {
        ui->status->setText(e.what());
    }
    if (new_status != status) {
        status = new_status;
        emit statusChanged(status);
    }
    emit filenameChanged(ui->file->text());
}

void TouchstoneImport::preventCollisionWithGroup(QButtonGroup *group, int id)
{
    for(unsigned int i=0;i<touchstone.ports();i++) {
        group->button(i)->setEnabled(true);
    }
    // change selection in second group and mark invalid
    group->button(id)->setEnabled(false);
    group->button(id)->setChecked(false);
    if (group->checkedId() == -1 || group->checkedId() == id) {
        for(int i=0;i<4;i++) {
            if(i == id) {
                continue;
            }
            if(group->button(i)->isEnabled()) {
                group->button(i)->setChecked(true);
                break;
            }
        }
    }
}
