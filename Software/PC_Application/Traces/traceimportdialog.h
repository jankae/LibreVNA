#ifndef TRACEIMPORTDIALOG_H
#define TRACEIMPORTDIALOG_H

#include <QDialog>
#include "tracemodel.h"
#include <vector>

namespace Ui {
class TraceImportDialog;
}

class TraceParameterModel : public QAbstractTableModel
{
    Q_OBJECT
    friend class TraceImportDialog;
public:
    TraceParameterModel(std::vector<Trace*> traces, QString prefix, QObject *parent = 0);
    ~TraceParameterModel(){};

    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    int columnCount(const QModelIndex &parent = QModelIndex()) const override;
    QVariant data(const QModelIndex &index, int role) const override;
    QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;
    bool setData(const QModelIndex &index, const QVariant &value, int role = Qt::EditRole) override;
    Qt::ItemFlags flags(const QModelIndex &index) const override;

    // adds all enabled traces to the model, deletes other traces
    std::vector<Trace *> import(TraceModel &model);
private:
    class Parameter {
    public:
        bool enabled;
        QString trace;
        QString name;
        QColor color;
    };
    std::vector<Parameter> params;
    std::vector<Trace*> traces;
};

class TraceImportDialog : public QDialog
{
    Q_OBJECT

public:
    explicit TraceImportDialog(TraceModel &model, std::vector<Trace*> traces, QString prefix = QString(), QWidget *parent = nullptr);
    ~TraceImportDialog();

private slots:
    void on_buttonBox_accepted();
    void on_tableView_doubleClicked(const QModelIndex &index);

signals:
    void importFinsished(const std::vector<Trace*> &traces);

private:
    Ui::TraceImportDialog *ui;
    TraceModel &model;
    TraceParameterModel *tableModel;
};

#endif // TRACEIMPORTDIALOG_H
