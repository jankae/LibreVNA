#include "windowfunction.h"
#define _USE_MATH_DEFINES
#include <math.h>
#include <QComboBox>
#include <QLabel>
#include <QFormLayout>
#include "CustomWidgets/siunitedit.h"
#include <QDebug>

QString WindowFunction::typeToName(WindowFunction::Type type)
{
    switch(type) {
    case Type::Rectangular: return "Rectangular"; break;
//    case Type::Kaiser: return "Kaiser"; break;
    case Type::Hamming: return "Hamming"; break;
    case Type::Hann: return "Hann"; break;
    case Type::Blackman: return "Blackman"; break;
    case Type::Gaussian: return "Gaussian"; break;
//    case Type::Chebyshev: return "Chebyshev"; break;
    default: return "Invalid"; break;
    }
}

WindowFunction::WindowFunction(WindowFunction::Type type)
{
    this->type = type;
    // set default parameters
    kaiser_alpha = 3.0;
    gaussian_sigma = 0.4;
    chebyshev_alpha = 5;
}

void WindowFunction::apply(std::vector<std::complex<double> > &data) const
{
    unsigned int N = data.size();
    for(unsigned int n = 0;n<N;n++) {
        data[n] *= getFactor(n, N);
    }
}

void WindowFunction::reverse(std::vector<std::complex<double> > &data) const
{
    unsigned int N = data.size();
    for(unsigned int n = 0;n<N;n++) {
        data[n] /= getFactor(n, N);
    }
}

QWidget *WindowFunction::createEditor()
{
    auto top = new QWidget();
    auto layout = new QFormLayout();
    top->setLayout(layout);
    auto cbType = new QComboBox();
    for(unsigned int i=0;i<(unsigned int) Type::Last;i++) {
        cbType->addItem(typeToName((Type) i));
    }

    layout->addRow(new QLabel("Type:"), cbType);

    QObject::connect(cbType, qOverload<int>(&QComboBox::currentIndexChanged), [=](int newIndex){
        if(layout->rowCount() > 1) {
            layout->removeRow(1);
        }
        type = (Type) newIndex;
        QLabel *paramLabel = nullptr;
        SIUnitEdit *paramEdit = nullptr;
        // add GUI elements for window types that have a parameter
        switch(type) {
        case Type::Gaussian:
            paramLabel = new QLabel("Parameter σ:");
            paramEdit = new SIUnitEdit("", " ", 3);
            paramEdit->setValue(gaussian_sigma);
            QObject::connect(paramEdit, &SIUnitEdit::valueChanged, [=](double newval) {
                gaussian_sigma = newval;
            });
            break;
//        case Type::Chebyshev:
//            paramLabel = new QLabel("Parameter α:");
//            paramEdit = new SIUnitEdit("", " ", 3);
//            paramEdit->setValue(chebyshev_alpha);
//            QObject::connect(paramEdit, &SIUnitEdit::valueChanged, [=](double newval) {
//                chebyshev_alpha = newval;
//            });
//            break;
//        case Type::Kaiser:
//            // TODO
//            break;
        default:
            break;
        }
        if(paramLabel != nullptr && paramEdit != nullptr) {
            layout->addRow(paramLabel, paramEdit);
            QObject::connect(paramEdit, &SIUnitEdit::valueChanged, this, &WindowFunction::changed);
        }
        emit changed();
    });

    cbType->setCurrentIndex((int) type);

    return top;
}

WindowFunction::Type WindowFunction::getType() const
{
    return type;
}

QString WindowFunction::getDescription()
{
    QString ret = typeToName(type);
    if(type == Type::Gaussian) {
        ret += ", σ=" + QString::number(gaussian_sigma);
//    } else if(type == Type::Chebyshev) {
//        ret += ", α=" + QString::number(chebyshev_alpha);
    }
    return ret;
}

nlohmann::json WindowFunction::toJSON()
{
    nlohmann::json j;
    j["type"] = typeToName(type).toStdString();
    // add additional parameter if type has one
    switch(type) {
    case Type::Gaussian:
        j["sigma"] = gaussian_sigma;
        break;
    default:
        break;
    }
    return j;
}

void WindowFunction::fromJSON(nlohmann::json j)
{
    qDebug() << "Setting window function from json";
    QString typeName = QString::fromStdString(j["type"]);
    unsigned int i=0;
    for(;i<(int) Type::Last;i++) {
        if(typeToName((Type) i) == typeName) {
            type = Type(i);
            break;
        }
    }
    if(i>=(int) Type::Last) {
        qWarning() << "Invalid window type specified, defaulting to hamming";
        type = Type::Hamming;
    }
    switch(type) {
    case Type::Gaussian:
        gaussian_sigma = j.value("sigma", 0.4);
        break;
    default:
        break;
    }
}

double WindowFunction::getFactor(unsigned int n, unsigned int N) const
{
    // all formulas from https://en.wikipedia.org/wiki/Window_function
    switch(type) {
    case Type::Rectangular:
        // nothing to do
        return 1.0;
//    case Type::Kaiser:
//        // TODO
//        break;
    case Type::Hamming:
        return 25.0/46.0 - (21.0/46.0) * cos(2*M_PI*n / N);
    case Type::Hann:
        return pow(sin(M_PI*n / N), 2.0);
    case Type::Blackman:
        return 0.42 - 0.5 * cos(2*M_PI*n / N) + 0.08 * cos(4*M_PI*n / N);
    case Type::Gaussian:
        return exp(-0.5 * pow((n - (double) N/2) / (gaussian_sigma * N / 2), 2));
//    case Type::Chebyshev: {
//        double beta = cosh(1.0 / N * acosh(pow(10, chebyshev_alpha)));
//        double T_N_arg = beta * cos(M_PI*n/(N+1));
//        double T_N;
//        if(T_N_arg >= 1.0) {
//            T_N = cosh(N * acosh(T_N_arg));
//        } else if(T_N_arg <= -1.0) {
//            T_N = pow(-1.0, N) * cosh(N * acosh(T_N_arg));
//        } else {
//            T_N = cos(N * acos(T_N_arg));
//        }
//        return T_N / pow(10.0, chebyshev_alpha);
//    }
    default:
        return 1.0;
    }
}
