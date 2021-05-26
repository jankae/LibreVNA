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
    void assignUI(QComboBox *cb, int pos, int width);
    void assignUI(QSpinBox *sb, int pos, int width);

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
    QString name;
    int address;
    int width;
    unsigned long value;
};

#endif // REGISTER_H
