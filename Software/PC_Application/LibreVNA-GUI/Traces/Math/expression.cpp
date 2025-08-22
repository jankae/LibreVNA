#include "expression.h"

#include "ui_expressiondialog.h"
#include "Traces/trace.h"
#include "ui_expressionexplanationwidget.h"
#include "appwindow.h"

#include <QWidget>
#include <QDebug>

using namespace mup;
using namespace std;

Math::Expression::Expression()
{
    parser = new ParserX(pckCOMMON | pckUNIT | pckCOMPLEX);
    parser->DefineVar("x", Variable(&x));
    dataType = DataType::Invalid;
    expressionChanged();
}

Math::Expression::~Expression()
{
    delete parser;
}

TraceMath::DataType Math::Expression::outputType(TraceMath::DataType inputType)
{
    return inputType;
}

QString Math::Expression::description()
{
    return "Custom expression: " + exp;
}

void Math::Expression::edit()
{
    auto d = new QDialog();
    auto ui = new Ui::ExpressionDialog;
    d->setAttribute(Qt::WA_DeleteOnClose);
    ui->setupUi(d);
    ui->expEdit->setText(exp);
    connect(ui->buttonBox, &QDialogButtonBox::accepted, [=](){
        exp = ui->expEdit->text();
        expressionChanged();
    });
    switch(dataType) {
    case DataType::Frequency: ui->stackedWidget->setCurrentIndex(0); break;
    case DataType::Time: ui->stackedWidget->setCurrentIndex(1); break;
    case DataType::Power: ui->stackedWidget->setCurrentIndex(2); break;
    case DataType::TimeZeroSpan: ui->stackedWidget->setCurrentIndex(3); break;
    default: break;
    }

    if(AppWindow::showGUI()) {
        d->show();
    }
}

QWidget *Math::Expression::createExplanationWidget()
{
    auto w = new QWidget();
    auto ui = new Ui::ExpressionExplanationWidget;
    ui->setupUi(w);
    connect(w, &QWidget::destroyed, [=](){
        delete ui;
    });
    return w;
}

nlohmann::json Math::Expression::toJSON()
{
    nlohmann::json j;
    j["exp"] = exp.toStdString();
    return j;
}

void Math::Expression::fromJSON(nlohmann::json j)
{
    exp = QString::fromStdString(j.value("exp", ""));
    expressionChanged();
}

void Math::Expression::inputSamplesChanged(unsigned int begin, unsigned int end)
{
    std::vector<Data> in;
    if(input) {
        in = input->getData();
    }
    dataMutex.lock();
    data.resize(in.size());
    // sanity check input values
    if(end > 0 && end > in.size()) {
        end = in.size();
    }
    if(end <= begin) {
        dataMutex.unlock();
        return;
    }
    try {
        for(unsigned int i=begin;i<end;i++) {
            t = in[i].x;
            f = in[i].x;
            P = in[i].x;
            w = in[i].x * 2 * M_PI;
            d = root()->timeToDistance(t);
            x = in[i].y;
            Value res = parser->Eval();
            data[i].x = in[i].x;
            data[i].y = res.GetComplex();
        }
        success();
    } catch (const ParserError &e) {
        error(QString::fromStdString(e.GetMsg()));
    }
    dataMutex.unlock();
    emit outputSamplesChanged(begin, end);
}

void Math::Expression::inputTypeChanged(DataType type)
{
    // call base class slot
    TraceMath::inputTypeChanged(type);
    // we need to evaluate the expression again to create the correct variables
    expressionChanged();
}

void Math::Expression::expressionChanged()
{
    if(exp.isEmpty()) {
        error("Empty expression");
        return;
    }
    parser->SetExpr(exp.toStdString());
    parser->RemoveVar("t");
    parser->RemoveVar("d");
    parser->RemoveVar("f");
    parser->RemoveVar("w");
    parser->RemoveVar("P");
    switch(dataType) {
    case DataType::Time:
        parser->DefineVar("t", Variable(&t));
        parser->DefineVar("d", Variable(&d));
        break;
    case DataType::Frequency:
        parser->DefineVar("f", Variable(&f));
        parser->DefineVar("w", Variable(&w));
        break;
    case DataType::Power:
        parser->DefineVar("P", Variable(&P));
        break;
    case DataType::TimeZeroSpan:
        parser->DefineVar("t", Variable(&t));
        break;
    default:
        break;
    }
    if(input) {
        inputSamplesChanged(0, input->numSamples());
    }
}
