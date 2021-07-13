#include "csvimport.h"
#include "ui_csvimport.h"
#include <QFileDialog>
#include "Traces/trace.h"
#include <QDebug>

CSVImport::CSVImport(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::csvimport),
    status(false)
{
    ui->setupUi(this);
    ui->from->setPrecision(5);
    ui->to->setPrecision(5);
    ui->traces->setPrecision(0);
    ui->points->setPrecision(0);
    connect(ui->file, &QLineEdit::textChanged, this, &CSVImport::setFile);
}

CSVImport::~CSVImport()
{
    delete ui;
}

bool CSVImport::getStatus()
{
    return status;
}

CSV CSVImport::getCSV()
{
    return csv;
}

QString CSVImport::getFilename()
{
    return ui->file->text();
}

bool CSVImport::fillTrace(Trace &t)
{
    try {
        t.fillFromCSV(csv, ui->traceSelector->currentIndex());
        return true;
    } catch (const std::exception &e) {
        qWarning() << QString("CSV import failed to fill trace: ") + e.what();
        return false;
    }
}

void CSVImport::setFile(QString filename)
{
    ui->file->setText(filename);
    evaluateFile();
    emit filenameChanged(filename);
}

void CSVImport::selectTrace(unsigned int index)
{
    ui->traceSelector->setCurrentIndex(index);
}

void CSVImport::on_browse_clicked()
{
    auto filename = QFileDialog::getOpenFileName(nullptr, "Open measurement file", "", "CSV files (*.csv)", nullptr, QFileDialog::DontUseNativeDialog);
    if (filename.length() > 0) {
        ui->file->setText(filename);
        evaluateFile();
    }
}

void CSVImport::evaluateFile()
{
    // store currently selected trace
    auto selectedTrace = ui->traceSelector->currentText();

    bool new_status = false;
    ui->traces->setText("");
    ui->points->setText("");
    ui->from->setText("");
    ui->to->setText("");
    ui->status->clear();
    ui->traceSelector->clear();
    try {
        csv = CSV::fromFile(ui->file->text());
        auto domain = Trace::dataTypeFromString(csv.getHeader(0));
        if (domain == Trace::DataType::Invalid) {
            // just assume frequency domain
            domain = Trace::DataType::Frequency;
        }
        ui->domain->setCurrentText(Trace::dataTypeToString(domain));
        switch(domain) {
        case Trace::DataType::Frequency:
            ui->from->setUnit("Hz");
            ui->to->setUnit("Hz");
            break;
        case Trace::DataType::Time:
            ui->from->setUnit("s");
            ui->to->setUnit("s");
            break;
        case Trace::DataType::Power:
            ui->from->setUnit("dBm");
            ui->to->setUnit("dBm");
            break;
        default:
            break;
        }

        auto xvalues = csv.getColumn(0);
        ui->from->setValue(xvalues.front());
        ui->to->setValue(xvalues.back());
        ui->points->setValue(xvalues.size());

        bool okay = true;
        int numTraces = 0;
        while(okay) {
            try {
                Trace t;
                auto name = t.fillFromCSV(csv, numTraces);
                ui->traceSelector->addItem(name);
                numTraces++;
            } catch (const std::exception &e) {
                // no more traces
                okay = false;
            }
        }
        ui->traces->setValue(numTraces);

        // check if previously selected trace is still available
        if(ui->traceSelector->findText(selectedTrace) >= 0) {
            ui->traceSelector->setCurrentText(selectedTrace);
        } else {
            ui->traceSelector->setCurrentIndex(0);
        }

        new_status = true;
    } catch (const std::exception &e) {
        ui->status->setText(e.what());
    }
    if (new_status != status) {
        status = new_status;
        emit statusChanged(status);
    }
    emit filenameChanged(ui->file->text());
}
