#include "jsonpickerdialog.h"
#include "ui_jsonpickerdialog.h"




JSONPickerDialog::JSONPickerDialog(const nlohmann::json &json, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::JSONPickerDialog),
    model(new JSONModel(json))
{
    ui->setupUi(this);
    ui->treeView->setModel(model);
}

JSONPickerDialog::~JSONPickerDialog()
{
    delete ui;
    delete model;
}

JSONModel::JSONModel(const nlohmann::json &json, QObject *parent) :
    json(json)
{
    Q_UNUSED(parent)
    setupJsonInfo(json);
}

JSONModel::~JSONModel()
{

}

QModelIndex JSONModel::index(int row, int column, const QModelIndex &parent) const
{
    if (!hasIndex(row, column, parent))
        return QModelIndex();

    nlohmann::json const *parentItem;

    if (!parent.isValid())
        parentItem = &json;
    else
        parentItem = static_cast<nlohmann::json*>(parent.internalPointer());

    auto it = parentItem->begin();
    int rb = row;
    while(rb) {
        it++;
        rb--;
    }
    nlohmann::json *childItem = const_cast<nlohmann::json*>(&*it);
    if (childItem)
        return createIndex(row, column, childItem);
    return QModelIndex();
}

QModelIndex JSONModel::parent(const QModelIndex &index) const
{
    if (!index.isValid())
        return QModelIndex();

    nlohmann::json *childItem = static_cast<nlohmann::json*>(index.internalPointer());
    if (childItem == &json) {
        return QModelIndex();
    }

    // find the parent of this entry and its position in the list
    nlohmann::json *parentItem = const_cast<nlohmann::json*>(jsonInfo.at(childItem).parent);
    auto it = parentItem->begin();
    int row = 0;
    while(&*it != childItem) {
        it++;
        row++;
    }
    return createIndex(row, 0, parentItem);
}

int JSONModel::rowCount(const QModelIndex &parent) const
{
    const nlohmann::json *parentItem;
    if (parent.column() > 0) {
        return 0;
    }

    if (!parent.isValid()) {
        parentItem = &json;
    } else {
        parentItem = static_cast<nlohmann::json*>(parent.internalPointer());
    }

    if (parentItem->is_object() || parentItem->is_array()) {
        return parentItem->size();
    } else {
        return 0;
    }
}

int JSONModel::columnCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent)
    return 2;
}

QVariant JSONModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid())
        return QVariant();

    nlohmann::json *item = static_cast<nlohmann::json*>(index.internalPointer());
    auto info = jsonInfo.at(item);
    switch(role) {
    case Qt::DisplayRole:
        switch(index.column()) {
        case 0:
            return info.name;
        case 1:
            if(item->is_object() || item->is_array()) {
                return QVariant();
            } else {
                return info.data;
            }
        }
    case Qt::CheckStateRole: {
        if(index.column() == 0) {
            return info.enabled ? Qt::Checked : Qt::Unchecked;
        } else {
            return QVariant();
        }
    }
    default:
        return QVariant();
    }
}

QVariant JSONModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    Q_UNUSED(section)
    Q_UNUSED(orientation)
    Q_UNUSED(role)
    return QVariant();
}

bool JSONModel::setData(const QModelIndex &index, const QVariant &value, int role)
{
    Q_UNUSED(value)
    nlohmann::json *item = static_cast<nlohmann::json*>(index.internalPointer());
    auto info = jsonInfo.at(item);
    if(role == Qt::CheckStateRole)
    {
        info.enabled = !info.enabled;
        jsonInfo[item] = info;
        emit dataChanged(index, index);
        return true;
    }

    if (role != Qt::EditRole)
        return false;
    return true;
}

Qt::ItemFlags JSONModel::flags(const QModelIndex &index) const
{
    if (!index.isValid()) {
        return Qt::NoItemFlags;
    }

    return Qt::ItemIsEditable | QAbstractItemModel::flags(index)|Qt::ItemIsUserCheckable;
}

void JSONModel::setupJsonInfo(const nlohmann::json &j)
{
    for(auto it = j.begin();it != j.end(); it++) {
        JSONInfo i;
        i.parent = &j;
        i.enabled = true;
        if(j.is_object()) {
            i.name = it.key().c_str();
        } else if(j.is_array()) {
            i.name = QString::number(it - j.begin() + 1);
        }
        if(it->is_object() || it->is_array()) {
            setupJsonInfo(*it);
        } else {
            i.data = QString::fromStdString(it->dump());
        }
        jsonInfo[&*it] = i;
    }
}
