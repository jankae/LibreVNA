#ifndef SIUNITEDIT_H
#define SIUNITEDIT_H

#include <QLineEdit>

class SIUnitEdit : public QLineEdit
{
    Q_OBJECT
public:
    SIUnitEdit(QString unit = QString(), QString prefixes = " ", int precision = 0, QWidget *parent = nullptr);
    SIUnitEdit(QWidget *parent);

    void setUnit(QString unit) { this->unit = unit; setValueQuiet(_value); }
    void setPrefixes(QString prefixes) { this->prefixes = prefixes; setValueQuiet(_value); }
    void setPrecision(int precision) { this->precision = precision; setValueQuiet(_value); }
    double value() { return _value; }
public slots:
    void setValue(double value);
    void setValueQuiet(double value);
signals:
    void valueChanged(double newvalue);
    void valueUpdated(QWidget *w);
    void editingAborted();
    void focusLost();
protected:
    bool eventFilter(QObject *obj, QEvent *event) override;
private slots:
    void continueEditing();
private:
    void parseNewValue(double factor);
    QString unit, prefixes;
    int precision;
    double _value;
};

#endif // SIUNITEDIT_H
