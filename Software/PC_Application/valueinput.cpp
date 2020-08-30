#include "valueinput.h"
#include <QGridLayout>
#include <QKeyEvent>
#include <QApplication>
#include <QDesktopWidget>

constexpr QSize ValueInput::minButtonSize;

ValueInput::ValueInput(std::vector<Unit> units, QString name, QString initialValue)
{
    auto fontButton = QFont("Arial", 12);
    auto fontLabel = QFont("Arial", 30);

    input = initialValue;
    // always use dots instead of comma
    input.replace(',', '.');
    this->units = units;
    setWindowTitle(name);

    // Create layout
    auto layout = new QGridLayout();

    // Add Label
    label = new QLabel();
    layout->addWidget(label, 0, 0, 2, 4);
    label->setText(input);
    label->setAlignment(Qt::AlignCenter);
    label->setFont(fontLabel);

    // Create buttons and add to layout
    for(int i=0;i<3;i++) {
        for(int j=0;j<3;j++) {
            int number = i+1 + j*3;
            auto name = QString::number(number);
            auto button = new QPushButton(name);
            button->setFont(fontButton);
            button->setMinimumSize(minButtonSize);
            connect(button, &QPushButton::clicked, [=]() {this->AddToInput(name);});
            layout->addWidget(button, j+2, i);
        }
    }
    bDot = new QPushButton(".");
    bDot->setFont(fontButton);
    bDot->setMinimumSize(minButtonSize);
    if(input.contains('.')) {
        bDot->setDisabled(true);
    }
    layout->addWidget(bDot, 5, 0);
    connect(bDot, &QPushButton::clicked, [=]() {this->AddToInput(".");});
    auto bZero = new QPushButton("0");
    bZero->setFont(fontButton);
    bZero->setMinimumSize(minButtonSize);
    connect(bZero, &QPushButton::clicked, [=]() {this->AddToInput("0");});
    layout->addWidget(bZero, 5, 1);
    auto bSign = new QPushButton("+/-");
    bSign->setFont(fontButton);
    bSign->setMinimumSize(minButtonSize);
    layout->addWidget(bSign, 5, 2);
    connect(bSign, SIGNAL(clicked()), this, SLOT(ChangeSign()));

    auto bDel = new QPushButton("Backspace");
    bDel->setMinimumSize(minButtonSize);
    layout->addWidget(bDel, 6, 0, 1, 3);
    connect(bDel, SIGNAL(clicked()), this, SLOT(Backspace()));

    auto bAbort = new QPushButton("Abort");
    bAbort->setMinimumSize(minButtonSize);
    layout->addWidget(bAbort, 6, 3);
    connect(bAbort, SIGNAL(clicked()), this, SLOT(Abort()));

    // Add unit inputs
    if(units.size() > 4) {
        units.resize(4);
    }
    for(unsigned int i=0;i<units.size();i++) {
        auto bUnit = new QPushButton(units[i].name);
        bUnit->setFont(fontButton);
        bUnit->setMinimumSize(minButtonSize);
        connect(bUnit, &QPushButton::clicked, [=](){this->UnitPressed(units[i].factor);});
        layout->addWidget(bUnit, i+2, 3);
    }

    setLayout(layout);
    setWindowModality(Qt::ApplicationModal);
    show();
}

void ValueInput::keyPressEvent(QKeyEvent *event) {
    auto key = event->key();
    if(key >= '0' && key <= '9') {
        AddToInput((QChar) key);
        event->accept();
    } else if((key == '.' || key == ',') && bDot->isEnabled()) {
        AddToInput(".");
        event->accept();
    } else if(key == Qt::Key_Escape) {
        Abort();
        event->accept();
    } else if(key == Qt::Key_Backspace) {
        Backspace();
        event->accept();
    } else if(key == '-') {
        ChangeSign();
        event->accept();
    }
}

void ValueInput::AddToInput(QString a)
{
    input.append(a);
    label->setText(input);
    if(a == '.') {
        bDot->setDisabled(true);
    }
}

void ValueInput::ChangeSign()
{
    if(input.at(0) == '-') {
        input.remove(0, 1);
    } else {
        input.prepend('-');
    }
    label->setText(input);
}

void ValueInput::Backspace()
{
    if(input.size() > 0) {
        if(input.at(input.size()-1) == '.') {
            bDot->setEnabled(true);
        }
        input.chop(1);
        label->setText(input);
    }
}

void ValueInput::Abort()
{
    close();
}

void ValueInput::UnitPressed(double factor)
{
    double value = input.toDouble() * factor;
    emit ValueChanged(value);
    close();
}
