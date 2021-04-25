#ifndef TRACEEDITDIALOG_H
#define TRACEEDITDIALOG_H

#include <QDialog>
#include "trace.h"
#include <QAbstractTableModel>

namespace Ui {
class TraceEditDialog;
}

class MathModel : public QAbstractTableModel
{
    Q_OBJECT
public:
    MathModel(Trace &t, QObject *parent = 0);

    enum {
        ColIndexStatus = 0,
        ColIndexDescription = 1,
        ColIndexDomain = 2,
        ColIndexLast,
    };

    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    int columnCount(const QModelIndex &parent = QModelIndex()) const override;
    QVariant data(const QModelIndex &index, int role) const override;
    QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;
    Qt::ItemFlags flags(const QModelIndex &index) const override;

    void addOperation(TraceMath *math);
    void addOperations(std::vector<TraceMath*> maths);
    void deleteRow(unsigned int row);

private:
    Trace &t;
};

class TraceEditDialog : public QDialog
{
    Q_OBJECT

public:
    explicit TraceEditDialog(Trace &t, QWidget *parent = nullptr);
    ~TraceEditDialog();

private slots:
    void on_buttonBox_accepted();

private:
    Ui::TraceEditDialog *ui;
    Trace &trace;
    bool VNAtrace;
};

#endif // TRACEEDITDIALOG_H
