#include "expression.h"
#include <QWidget>
#include "ui_expressiondialog.h"
#include <QWidget>
#include <QDebug>
#include "Traces/trace.h"
#include "ui_expressionexplanationwidget.h"

using namespace mup;
using namespace std;

Math::Expression::Expression()
{
    parser = new ParserX(pckCOMMON | pckUNIT | pckCOMPLEX);
    parser->DefineVar("x", Variable(&x));
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
    ui->setupUi(d);
    ui->expEdit->setText(exp);
    connect(ui->buttonBox, &QDialogButtonBox::accepted, [=](){
        exp = ui->expEdit->text();
        expressionChanged();
    });
    if(dataType == DataType::Time) {
        // select the label explaining the time domain variables (frequency label is the default)
        ui->stackedWidget->setCurrentIndex(1);
    }
    d->show();
}

QWidget *Math::Expression::createExplanationWidget()
{
    auto w = new QWidget();
    auto ui = new Ui::ExpressionExplanationWidget;
    ui->setupUi(w);
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
    auto in = input->rData();
    data.resize(in.size());
    try {
        for(unsigned int i=begin;i<end;i++) {
            t = in[i].x;
            f = in[i].x;
            w = in[i].x * 2 * M_PI;
            d = root()->timeToDistance(t);
            x = in[i].y;
            Value res = parser->Eval();
            data[i].x = in[i].x;
            data[i].y = res.GetComplex();
        }
        success();
        emit outputSamplesChanged(begin, end);
    } catch (const ParserError &e) {
        error(QString::fromStdString(e.GetMsg()));
    }
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
    switch(dataType) {
    case DataType::Time:
        parser->DefineVar("t", Variable(&t));
        parser->DefineVar("d", Variable(&d));
        break;
    case DataType::Frequency:
        parser->DefineVar("f", Variable(&f));
        parser->DefineVar("w", Variable(&w));
        break;
    default:
        break;
    }
    if(input) {
        inputSamplesChanged(0, input->rData().size());
    }
}
