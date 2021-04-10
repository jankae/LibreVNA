#ifndef JSONPICKERDIALOG_H
#define JSONPICKERDIALOG_H

#include <QDialog>
#include <QAbstractItemModel>
#include "json.hpp"

namespace Ui {
class JSONPickerDialog;
}

class JSONModel : public QAbstractItemModel
{
    Q_OBJECT
public:
    JSONModel(const nlohmann::json &json, QObject *parent = 0);
    ~JSONModel();

    QModelIndex index(int row, int column, const QModelIndex &parent = QModelIndex()) const override;
    QModelIndex parent(const QModelIndex &index) const override;
    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    int columnCount(const QModelIndex &parent = QModelIndex()) const override;
    QVariant data(const QModelIndex &index, int role) const override;
    QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;
    bool setData(const QModelIndex &index, const QVariant &value, int role = Qt::EditRole) override;
    Qt::ItemFlags flags(const QModelIndex &index) const override;

private:
    void setupJsonInfo(const nlohmann::json &j);
    const nlohmann::json &json;
    class JSONInfo {
    public:
        const nlohmann::json *parent;
        QString name;
        QString data;
        bool enabled;
    };

    std::map<const nlohmann::json*, JSONInfo> jsonInfo;
};

class JSONPickerDialog : public QDialog
{
    Q_OBJECT

public:
    explicit JSONPickerDialog(const nlohmann::json &json, QWidget *parent = nullptr);
    ~JSONPickerDialog();

private:
    Ui::JSONPickerDialog *ui;
    JSONModel *model;
};

#endif // JSONPICKERDIALOG_H
