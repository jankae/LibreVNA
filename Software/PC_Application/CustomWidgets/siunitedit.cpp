#include "siunitedit.h"

#include <QDoubleValidator>
#include <unit.h>
#include <QEvent>
#include <QKeyEvent>
#include <QDebug>
#include <QTimer>
#include <cmath>

SIUnitEdit::SIUnitEdit(QString unit, QString prefixes, int precision, QWidget *parent)
    : QLineEdit(parent)
{
    this->unit = unit;
    this->prefixes = prefixes;
    this->precision = precision;
    setAlignment(Qt::AlignCenter);
    installEventFilter(this);
    connect(this, &QLineEdit::editingFinished, [this]() {
       parseNewValue(1.0);
    });
    setValueQuiet(0);
}

SIUnitEdit::SIUnitEdit(QWidget *parent)
    : SIUnitEdit("", " ", 4, parent)
{

}

void SIUnitEdit::setValue(double value)
{
    if(value != _value) {
        setValueQuiet(value);
        emit valueChanged(value);
        emit valueUpdated(this);
    }
}

static char swapUpperLower(char c) {
    if(isupper(c)) {
        return tolower(c);
    } else if(islower(c)) {
        return toupper(c);
    } else {
        return c;
    }
}

bool SIUnitEdit::eventFilter(QObject *, QEvent *event)
{
    if (event->type() == QEvent::KeyPress) {
        int key = static_cast<QKeyEvent *>(event)->key();
        if(key == Qt::Key_Escape) {
            // abort editing process and set old value
            clear();
            setValueQuiet(_value);
            emit editingAborted();
            clearFocus();
            return true;
        }
        if(key == Qt::Key_Return) {
            // use new value without prefix
           parseNewValue(1.0);
           continueEditing();
           return true;
        }
        auto mod = static_cast<QKeyEvent *>(event)->modifiers();
        if (!(mod & Qt::ShiftModifier)) {
            key = tolower(key);
        }
        if(key <= 255) {
            if (prefixes.indexOf(key) >= 0) {
                // a valid prefix key was pressed
                parseNewValue(Unit::SIPrefixToFactor(key));
                continueEditing();
                return true;
            } else if (prefixes.indexOf(swapUpperLower(key)) >= 0) {
                // no match on the pressed case but on the upper/lower case instead -> also accept this
                parseNewValue(Unit::SIPrefixToFactor(swapUpperLower(key)));
                continueEditing();
                return true;
            }
        }
    } else if(event->type() == QEvent::FocusOut) {
        parseNewValue(1.0);
        emit focusLost();
    } else if(event->type() == QEvent::FocusIn) {
        // online found clumsy way to select all text when clicked!?!
        // just selectAll() alone does _not_ work!
        QTimer::singleShot(0, this, &SIUnitEdit::continueEditing);
    } else if(event->type() == QEvent::Wheel) {
        if(_value == 0.0) {
            // can't figure out step size with zero value
            return true;
        }
        auto wheel = static_cast<QWheelEvent*>(event);
        // most mousewheel have 15 degree increments, the reported delta is in 1/8th degree -> 120
        auto increment = wheel->angleDelta().y() / 120.0;
        // round toward bigger step in case of special higher resolution mousewheel
        unsigned int steps = std::abs(increment > 0 ? ceil(increment) : floor(increment));
        int sign = increment > 0 ? 1 : -1;
        // figure out step increment
        auto newVal = _value;
        if(hasFocus()) {
            auto cursor = cursorPosition();
            if(cursor == 0) {
                // cursor in front of first digit, do nothing (too big of a change, probably over/underflows)
                return true;
            }
            // change the digit at the current cursor
            int nthDigit = cursor;
            // account for decimal point/leading zero/sign
            if(_value < 0) {
                nthDigit--;
            }
            auto dotPos = text().indexOf('.');
            if(dotPos >= 0 && dotPos < nthDigit) {
                nthDigit--;
            }
            if(text().startsWith("-0.") || text().startsWith("0.")) {
                nthDigit--;
            }
            auto step_size = pow(10, floor(log10(std::abs(newVal))) - nthDigit + 1);
            newVal += step_size * steps * sign;
            setValue(newVal);
            setText(placeholderText());
            setCursorPosition(cursor);
        } else {
            // default to the third digit
            constexpr int nthDigit = 3;
            while(steps > 0) {
                // do update in multiple steps because the step size could change inbetween
                auto step_size = pow(10, floor(log10(std::abs(newVal))) - nthDigit + 1);
                newVal += step_size * sign;
                steps--;
            }
            setValue(newVal);
        }
        return true;
    }
    return false;
}

void SIUnitEdit::setValueQuiet(double value)
{
    _value = value;
    setPlaceholderText(Unit::ToString(value, unit, prefixes, precision));
}

void SIUnitEdit::parseNewValue(double factor)
{
    QString input = text();
    if(input.isEmpty()) {
        setValueQuiet(_value);
        emit editingAborted();
    } else {
        // remove optional unit
        if(input.endsWith(unit)) {
            input.chop(unit.size());
        }
        auto lastChar = input.at(input.size()-1).toLatin1();
        if(prefixes.indexOf(lastChar) >= 0) {
            factor = Unit::SIPrefixToFactor(lastChar);
            input.chop(1);
        }
        // remaining input should only contain numbers
        bool conversion_ok;
        auto v = input.toDouble(&conversion_ok);
        clear();
        if(conversion_ok) {
            setValue(v * factor);
        } else {
            qWarning() << "SIUnit conversion failure:" << input;
        }
    }
}

void SIUnitEdit::continueEditing()
{
    setText(placeholderText());
    selectAll();
}
