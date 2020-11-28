#ifndef TRACEMATHEDITDIALOG_H
#define TRACEMATHEDITDIALOG_H

#include <QDialog>
#include <QAbstractTableModel>
#include "Traces/trace.h"

namespace Ui {
class TraceMathEditDialog;
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
    void deleteRow(unsigned int row);
    void rowsSwapped(unsigned int top);

private:
    Trace &t;
};

class TraceMathEditDialog : public QDialog
{
    Q_OBJECT

public:
    explicit TraceMathEditDialog(Trace &t, QWidget *parent = nullptr);
    ~TraceMathEditDialog();

private:
    Ui::TraceMathEditDialog *ui;
};

#endif // TRACEMATHEDITDIALOG_H
