#ifndef TRACECSVEXPORT_H
#define TRACECSVEXPORT_H

#include <QDialog>
#include "tracemodel.h"

namespace Ui {
class TraceCSVExport;
}

class TraceCSVModel : public QAbstractListModel
{
    Q_OBJECT
public:
    TraceCSVModel(std::vector<Trace*> traces, QObject *parent = 0);
    ~TraceCSVModel(){};

    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    QVariant data(const QModelIndex &index, int role) const override;
    bool setData(const QModelIndex &index, const QVariant &value, int role = Qt::EditRole) override;
    Qt::ItemFlags flags(const QModelIndex &index) const override;

    std::vector<Trace*> tracesToExport();
signals:
    void selectionChanged(bool anySelected);
private:
    // check which traces can be exported (only allow traces on the same domain with identical span/time)
    void updateEnabledTraces();
    unsigned int points;
    double minX, maxX;
    Trace::DataType type;
    std::vector<Trace*> traces;
    std::vector<bool> enabled;
    std::vector<bool> save;
};

class TraceCSVExport : public QDialog
{
    Q_OBJECT

public:
    explicit TraceCSVExport(TraceModel &model, QWidget *parent = nullptr);
    ~TraceCSVExport();

private slots:
    void on_buttonBox_accepted();

private:
    Ui::TraceCSVExport *ui;
    TraceCSVModel model;
};

#endif // TRACECSVEXPORT_H
