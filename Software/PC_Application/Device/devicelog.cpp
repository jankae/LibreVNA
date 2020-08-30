#include "devicelog.h"
#include "ui_devicelog.h"
#include <QScrollBar>
#include <QFileDialog>
#include <fstream>

using namespace std;

DeviceLog::DeviceLog(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::DeviceLog)
{
    ui->setupUi(this);
    connect(ui->bClear, &QPushButton::clicked, this, &DeviceLog::clear);
}

DeviceLog::~DeviceLog()
{
    delete ui;
}

void DeviceLog::addLine(QString line)
{
    // Set color depending on log level
    QColor color = Qt::black;
    if(line.contains(",CRT]")) {
        color = Qt::red;
    } else if(line.contains(",ERR]")) {
        color = QColor("orange");
    } else if(line.contains(",WRN]")) {
        color = Qt::darkYellow;
    } else if(line.contains(",DBG")) {
        color = Qt::gray;
    }
    QTextCharFormat tf;
    tf = ui->text->currentCharFormat();
    tf.setForeground(QBrush(color));
    ui->text->setCurrentCharFormat(tf);
    ui->text->appendPlainText(line);
    if(ui->cbAutoscroll->isChecked()) {
        QScrollBar *sb = ui->text->verticalScrollBar();
        sb->setValue(sb->maximum());
    }
}

void DeviceLog::clear()
{
    ui->text->clear();
}

void DeviceLog::on_bToFile_clicked()
{
    auto filename = QFileDialog::getSaveFileName(this, "Select file for device log", "", "", nullptr, QFileDialog::DontUseNativeDialog);
    if(filename.length() > 0) {
        // create file
        ofstream file;
        file.open(filename.toStdString());
        file << ui->text->toPlainText().toStdString();
        file.close();
    }
}
