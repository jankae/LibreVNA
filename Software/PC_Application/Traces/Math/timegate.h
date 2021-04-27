#ifndef TIMEGATE_H
#define TIMEGATE_H

#include "tracemath.h"
#include "windowfunction.h"

namespace Math {

class TimeGate;

class TimeGateGraph : public QWidget {
public:
    TimeGateGraph(QWidget *parent);

    void setGate(TimeGate *gate) {
        this->gate = gate;
    }
private:
    static constexpr int catchDistance = 5;

    QPoint plotValueToPixel(double x, double y);
    QPointF pixelToPlotValue(QPoint p);
    void paintEvent(QPaintEvent *event) override;
    void mousePressEvent(QMouseEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;
    TimeGate *gate;
    bool grabbedStart;
    bool grabbedStop;
};

class TimeGate : public TraceMath
{
    Q_OBJECT
public:
    TimeGate();

    virtual DataType outputType(DataType inputType) override;
    virtual QString description() override;

    virtual void edit() override;

    static QWidget *createExplanationWidget();

    virtual nlohmann::json toJSON() override;
    virtual void fromJSON(nlohmann::json j) override;
    Type getType() override {return Type::TimeGate;};

    const std::vector<double> &rFilter() { return filter;};

    double getStart();
    double getStop();

public slots:
    // a single value of the input data has changed, index determines which sample has changed
    virtual void inputSamplesChanged(unsigned int begin, unsigned int end) override;

    void setStart(double start);
    void setStop(double stop);
    void setCenter(double center);
    void setSpan(double span);

private slots:
    void updateFilter();
signals:
    void filterUpdated();
    void startChanged(double newval);
    void stopChanged(double newval);
    void centerChanged(double newval);
    void spanChanged(double newval);
private:
    enum class Filter {
        None,
        Hamming,
        Hann
    };

    bool bandpass;
    double center, span;
    WindowFunction window;
    std::vector<double> filter;
};

}

#endif // TIMEGATE_H
