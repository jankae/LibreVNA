#include "devicetcpdriver.h"

#include "ui_devicetcpdriversettings.h"

#include <QLineEdit>

DeviceTCPDriver::DeviceTCPDriver(QString driverName)
{
    specificSettings.push_back(Savable::SettingDescription(&searchAddressString, driverName+".searchAddressString", ""));
}

QWidget *DeviceTCPDriver::createSettingsWidget()
{
    parseSearchAddressString();
    auto w = new QWidget();
    auto ui = new Ui::DeviceTCPDriverSettingsWidget;
    ui->setupUi(w);

    ui->list->clear();
    for(auto &a : searchAddresses) {
        auto item = new QListWidgetItem();
        if(a.isNull()) {
            item->setText("0.0.0.0");
        } else {
            item->setText(a.toString());
        }
        item->setFlags(item->flags() | Qt::ItemIsEditable);
        ui->list->addItem(item);
    }

    connect(ui->list->itemDelegate(), &QAbstractItemDelegate::commitData, this, [=](QWidget *pLineEdit) {
        QString strNewText = reinterpret_cast<QLineEdit*>(pLineEdit)->text();
        int nRow = ui->list->currentRow();
        auto address = QHostAddress(strNewText);
        if(!address.isNull()) {
            // valid IP
            searchAddresses[nRow] = address;
            updateSearchAddressString();
        }
        ui->list->item(nRow)->setText(searchAddresses[nRow].toString());
    });

    connect(ui->add, &QPushButton::clicked, [=](){
        searchAddresses.push_back(QHostAddress("0.0.0.0"));
        auto item = new QListWidgetItem(searchAddresses.back().toString());
        item->setFlags(item->flags() | Qt::ItemIsEditable);
        ui->list->addItem(item);
        updateSearchAddressString();
    });

    connect(ui->remove, &QPushButton::clicked, [=](){
        auto index = ui->list->currentRow();
        if(index >= 0 && index < (int) searchAddresses.size()) {
            delete ui->list->takeItem(index);
            searchAddresses.erase(searchAddresses.begin() + index);
            updateSearchAddressString();
        }
    });

    return w;
}

std::vector<QHostAddress> DeviceTCPDriver::getSearchAddresses()
{
    parseSearchAddressString();
    return searchAddresses;
}

void DeviceTCPDriver::parseSearchAddressString()
{
    QStringList list = searchAddressString.split(",");
    searchAddresses.clear();
    for(auto &l : list) {
        searchAddresses.push_back(QHostAddress(l));
    }
}

void DeviceTCPDriver::updateSearchAddressString()
{
    searchAddressString = "";
    for(auto &a : searchAddresses) {
        searchAddressString += a.toString();
        searchAddressString += ",";
    }
    if(searchAddressString.length() > 0) {
        // remove trailing comma
        searchAddressString.chop(1);
    }
}
