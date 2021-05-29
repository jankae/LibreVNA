#ifndef REGISTER_H
#define REGISTER_H

#include <QObject>
#include <QCheckBox>
#include <QComboBox>
#include <QSpinBox>
#include <QTableWidget>

class Register : public QObject
{
    Q_OBJECT
public:
    Register(QString name, int address, int width);

    void assignUI(QCheckBox *cb, int bitpos, bool inverted = false);
    // pos: bitpos at which the field in the register starts which is associated with the UI element
    // width: amount of bits in the field
    // ui_bitoffset: set to LSB of the ui element in this register field (only required for elements that
    // are not represented as one field in a register, e.g. when splitting bits across multiple registers)
    void assignUI(QComboBox *cb, int pos, int width, int ui_bitoffset = 0);
    void assignUI(QSpinBox *sb, int pos, int width, int ui_bitoffset = 0);

    // same as above but by specifying the register address instead of using the register directly
    static void assignUI(std::vector<Register*> regs, int address, QCheckBox *cb, int bitpos, bool inverted = false);
    static void assignUI(std::vector<Register*> regs, int address, QComboBox *cb, int pos, int width, int ui_bitoffset = 0);
    static void assignUI(std::vector<Register*> regs, int address, QSpinBox *sb, int pos, int width, int ui_bitoffset = 0);

    QString hexString();
    bool setFromString(QString hex);
    unsigned long getValue();
    unsigned long getValue(int pos, int width);
    QString getName() const;

    static void fillTableWidget(QTableWidget *w, std::vector<Register*> regs);

    int getAddress() const;

public slots:
    void setValue(unsigned long newval);
    void setValue(unsigned long newval, int pos, int width);
signals:
    void valueChanged(unsigned long newval);
private:
    static Register *findByAddress(std::vector<Register*> regs, int address);
    QString name;
    int address;
    int width;
    unsigned long value;
    bool updating; // for preventing endless recursion when updating register/its UI connections
};

#endif // REGISTER_H
