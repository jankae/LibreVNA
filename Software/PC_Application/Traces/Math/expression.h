#ifndef EXPRESSION_H
#define EXPRESSION_H

#include "tracemath.h"
#include "parser/mpParser.h"

namespace Math {

class Expression : public TraceMath
{
public:
    Expression();
    ~Expression();

    DataType outputType(DataType inputType) override;
    QString description() override;
    void edit() override;

    static QWidget* createExplanationWidget();

    virtual nlohmann::json toJSON() override;
    virtual void fromJSON(nlohmann::json j) override;
    Type getType() override {return Type::Expression;};

public slots:
    void inputSamplesChanged(unsigned int begin, unsigned int end) override;

private slots:
    void expressionChanged();
private:
    QString exp;
    mup::ParserX *parser;
    mup::Value t, d, f, w, x;
};

}

#endif // EXPRESSION_H
