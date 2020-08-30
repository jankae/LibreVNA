#ifndef VALUEINPUT_H
#define VALUEINPUT_H

#include <QWidget>
#include <vector>
#include <QPushButton>
#include <QLabel>
#include <QSignalMapper>

class ValueInput : public QWidget
{
    Q_OBJECT
public:
    class Unit {
    public:
        Unit(){};
        Unit(QString name, double factor):
            name(name), factor(factor){};
        QString name;
        double factor;
    };
    ValueInput(std::vector<Unit> units, QString name = "Value input", QString initialValue = QString());

signals:
    void ValueChanged(double value);

protected:
    void keyPressEvent(QKeyEvent *event) override;

private slots:
    void AddToInput(QString a);
    void ChangeSign();
    void Backspace();
    void Abort();
    void UnitPressed(double factor);
private:
    static constexpr QSize minButtonSize = QSize(50, 50);
    QString input;
    QLabel *label;
    QPushButton *bDot;
    std::vector<Unit> units;
};

#endif // VALUEINPUT_H
