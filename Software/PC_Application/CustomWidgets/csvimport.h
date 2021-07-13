#ifndef CSVIMPORT_H
#define CSVIMPORT_H

#include <QWidget>
#include "csv.h"
#include "Traces/trace.h"

namespace Ui {
class csvimport;
}

class CSVImport : public QWidget
{
    Q_OBJECT

public:
    explicit CSVImport(QWidget *parent = nullptr);
    ~CSVImport();

    bool getStatus();
    CSV getCSV();
    QString getFilename();

    bool fillTrace(Trace &t);

signals:
    void statusChanged(bool status);
    void filenameChanged(QString name);

public slots:
    void setFile(QString filename);
    void selectTrace(unsigned int index);

private slots:
    void on_browse_clicked();

private:
    void evaluateFile();
    Ui::csvimport *ui;
    int required_ports;
    CSV csv;
    bool status;
};

#endif // CSVIMPORT_H
