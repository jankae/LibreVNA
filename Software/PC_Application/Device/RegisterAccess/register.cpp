#include "register.h"
#include <exception>
#include <QHeaderView>
#include <QDebug>

Register::Register(QString name, int address, int width)
    : QObject(nullptr),
      name(name),
      address(address),
      width(width),
      updating(false)
{
    value = 0;
}

void Register::assignUI(QCheckBox *cb, int bitpos, bool inverted)
{
    connect(this, &Register::valueChanged, [=](unsigned int newval) {
        bool bit = newval & (1UL << bitpos);
        if(inverted) {
            bit = !bit;
        }
        cb->setChecked(bit);
    });
    connect(cb, &QCheckBox::toggled, [=](bool checked){
        if(inverted) {
            checked = !checked;
        }
        setValue(checked, bitpos, 1);
    });
}

void Register::assignUI(QComboBox *cb, int pos, int width, int ui_bitoffset)
{
    connect(this, &Register::valueChanged, [=]() {
        auto value = getValue(pos, width);
        auto mask = (1UL << width) - 1;
        mask <<= ui_bitoffset;
        value <<= ui_bitoffset;
        auto old_gui = cb->currentIndex();
        old_gui &= ~mask;
        old_gui |= value;
        if(cb->count() > static_cast<int>(old_gui)) {
            updating = true;
            cb->setCurrentIndex(old_gui);
            updating = false;
        }
    });
    connect(cb, qOverload<int>(&QComboBox::currentIndexChanged), [=](int index){
        if(!updating) {
            index >>= ui_bitoffset;
            setValue(index, pos, width);
        }
    });
}

void Register::assignUI(QSpinBox *sb, int pos, int width, int ui_bitoffset)
{
    connect(this, &Register::valueChanged, [=]() {
        auto value = getValue(pos, width);
        auto mask = (1UL << width) - 1;
        mask <<= ui_bitoffset;
        value <<= ui_bitoffset;
        auto old_gui = sb->value();
        old_gui &= ~mask;
        old_gui |= value;
        updating = true;
        sb->setValue(old_gui);
        updating = false;
    });
    connect(sb, qOverload<int>(&QSpinBox::valueChanged), [=](int index){
        if(!updating) {
            index >>= ui_bitoffset;
            setValue(index, pos, width);
        }
    });
}

void Register::assignUI(std::vector<Register *> regs, int address, QCheckBox *cb, int bitpos, bool inverted)
{
    auto reg = findByAddress(regs, address);
    if(reg) {
        reg->assignUI(cb, bitpos, inverted);
    }
}

void Register::assignUI(std::vector<Register *> regs, int address, QComboBox *cb, int pos, int width, int ui_bitoffset)
{
    auto reg = findByAddress(regs, address);
    if(reg) {
        reg->assignUI(cb, pos, width, ui_bitoffset);
    }

}

void Register::assignUI(std::vector<Register *> regs, int address, QSpinBox *sb, int pos, int width, int ui_bitoffset)
{
    auto reg = findByAddress(regs, address);
    if(reg) {
        reg->assignUI(sb, pos, width, ui_bitoffset);
    }

}

QString Register::hexString()
{
    return "0x" + QString("%1").arg(value, (width-1)/4 + 1, 16, QChar('0'));
}

bool Register::setFromString(QString s)
{
    bool okay;
    auto num = s.toULong(&okay, 0);
    if(okay) {
        setValue(num);
    }
    return okay;
}

unsigned long Register::getValue()
{
    return value;
}

unsigned long Register::getValue(int pos, int width)
{
    unsigned long mask = (1UL << width) - 1;
    mask <<= pos;
    auto masked = value & mask;
    masked >>= pos;
    return masked;
}

void Register::setValue(unsigned long newval)
{
    setValue(newval, 0, width);
}

void Register::setValue(unsigned long newval, int pos, int width)
{
    unsigned long mask = (1UL << width) - 1;
    newval &= mask;
    newval <<= pos;
    mask <<= pos;
    auto oldval = value;
    value &= ~mask;
    value |= newval;
    if(newval != oldval) {
        emit valueChanged(value);
    }
}

Register *Register::findByAddress(std::vector<Register *> regs, int address)
{
    for(auto r : regs) {
        if(r->getAddress() == address) {
            return r;
        }
    }
    return nullptr;
}

int Register::getAddress() const
{
    return address;
}

QString Register::getName() const
{
    return name;
}

void Register::fillTableWidget(QTableWidget *l, std::vector<Register *> regs)
{
    l->clear();
    l->setRowCount(regs.size());
    l->setColumnCount(3);
    l->setHorizontalHeaderLabels({"Name", "Address", "Value"});
    l->verticalHeader()->setVisible(false);
    l->horizontalHeader()->setStretchLastSection(true);
//    l->setSortingEnabled(true);
    for(unsigned int i=0;i<regs.size();i++) {
        l->setItem(i, 0, new QTableWidgetItem(regs[i]->getName()));
        l->item(i, 0)->setFlags(l->item(i, 0)->flags() &= ~Qt::ItemIsEditable);
        auto address = "0x" + QString::number(regs[i]->address, 16);
        l->setItem(i, 1, new QTableWidgetItem(address));
        l->item(i, 1)->setFlags(l->item(i, 1)->flags() &= ~Qt::ItemIsEditable);
        auto valueItem = new QTableWidgetItem;
        valueItem->setFlags(valueItem->flags() | Qt::ItemIsEditable);
        valueItem->setData(Qt::DisplayRole, regs[i]->hexString());
        l->setItem(i, 2, valueItem);
        connect(regs[i], &Register::valueChanged, [=]() {
            l->item(i, 2)->setData(Qt::DisplayRole, regs[i]->hexString());
        });
    }
    connect(l, &QTableWidget::cellChanged, [=](int row, int column){
        if(column == 2) {
            auto reg = regs[row];
            QString input = l->item(row, column)->data(Qt::DisplayRole).toString();
            reg->setFromString(input);
            l->item(row, column)->setData(Qt::DisplayRole, reg->hexString());
        }
    });
}

