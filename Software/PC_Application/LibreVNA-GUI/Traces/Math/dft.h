#ifndef DFT_H
#define DFT_H

#include "tracemath.h"
#include "windowfunction.h"

#include <QThread>
#include <QSemaphore>

namespace Math {

class DFT;

class DFTThread : public QThread
{
    Q_OBJECT
public:
    DFTThread(DFT &dft);
    ~DFTThread(){};
private:
    void run() override;
    DFT &dft;
};

class DFT : public TraceMath
{
    friend class DFTThread;
    Q_OBJECT
public:
    DFT();
    ~DFT();

    DataType outputType(DataType inputType) override;
    QString description() override;
    void edit() override;

    static QWidget* createExplanationWidget();

    virtual nlohmann::json toJSON() override;
    virtual void fromJSON(nlohmann::json j) override;
    Type getType() override {return Type::DFT;};

public slots:
    void inputSamplesChanged(unsigned int begin, unsigned int end) override;

private:
    void updateDFT();
    void clearOutput();
    bool automaticDC;
    double DCfreq;
    WindowFunction window;
    bool removePaddingFromTDR;
    bool revertWindowFromTDR;
    DFTThread *thread;
    bool destructing;
    QSemaphore semphr;
};

}

#endif // DFT_H
