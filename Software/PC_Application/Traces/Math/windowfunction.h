#ifndef WINDOWFUNCTION_H
#define WINDOWFUNCTION_H

#include <QWidget>
#include <complex>
#include <vector>
#include "savable.h"

class WindowFunction : public QObject, public Savable
{
    Q_OBJECT;
public:   
    enum class Type {
        Rectangular,
//        Kaiser,
        Gaussian,
//        Chebyshev,
        Hann,
        Hamming,
        Blackman,
        // always has to be the last entry
        Last,
    };
    static QString typeToName(Type type);

    WindowFunction(Type type = Type::Hamming);

    void apply(std::vector<std::complex<double>>& data) const;
    void reverse(std::vector<std::complex<double>>& data) const;

    QWidget *createEditor();

    Type getType() const;
    QString getDescription();

    virtual nlohmann::json toJSON() override;
    virtual void fromJSON(nlohmann::json j) override;

signals:
    void changed();

private:
    double getFactor(unsigned int n, unsigned int N) const;
    Type type;
    // parameters for the different types. Not all windows use one and most only one.
    // But keeping all parameters for all windows allows switching between window types
    // while remembering the settings for each type
    double kaiser_alpha;
    double gaussian_sigma;
    double chebyshev_alpha;
};

#endif // WINDOWFUNCTION_H
