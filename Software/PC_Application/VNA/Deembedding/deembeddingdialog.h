#ifndef DEEMBEDDINGDIALOG_H
#define DEEMBEDDINGDIALOG_H

#include <QDialog>
#include "deembeddingoption.h"
#include "deembedding.h"
#include <QAbstractTableModel>

namespace Ui {
class DeembeddingDialog;
}

class OptionModel : public QAbstractListModel
{
    Q_OBJECT
public:
    OptionModel(Deembedding *d, QObject *parent = 0);

    enum {
        ColIndexStatus = 0,
        ColIndexDescription = 1,
        ColIndexDomain = 2,
        ColIndexLast,
    };

    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    QVariant data(const QModelIndex &index, int role) const override;

    void addOption(DeembeddingOption *option);
    void deleteRow(unsigned int row);

private:
    Deembedding *d;
};

class DeembeddingDialog : public QDialog
{
    Q_OBJECT

public:
    explicit DeembeddingDialog(Deembedding* d, QWidget *parent = nullptr);
    ~DeembeddingDialog();

private:
    Ui::DeembeddingDialog *ui;
    OptionModel model;
};

#endif // DEEMBEDDINGDIALOG_H
