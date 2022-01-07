#include "util.h"

#include <QVector2D>

void Util::unwrapPhase(std::vector<double> &phase)
{
    for (unsigned int i = 1; i < phase.size(); i++) {
        double d = phase[i] - phase[i-1];
        d = d > M_PI ? d - 2 * M_PI : (d < -M_PI ? d + 2 * M_PI : d);
        phase[i] = phase[i-1] + d;
    }
}

void Util::linearRegression(const std::vector<double> &input, double &B_0, double &B_1)
{
    double x_mean = (input.size() - 1.0) / 2.0;
    double y_mean = std::accumulate(input.begin(), input.end(), 0.0) / input.size();
    double ss_xy = 0.0;
    for(unsigned int i=0;i<input.size();i++) {
        ss_xy += input[i] * i;
    }
    ss_xy -= input.size() * x_mean * y_mean;
    int n = input.size() - 1;
    double ss_xx = (1.0/6.0) * n * (n + 1) * (2*n + 1) - input.size() * x_mean * x_mean;

    B_1 = ss_xy / ss_xx;
    B_0 = y_mean - B_1 * x_mean;
}

double Util::distanceToLine(QPointF point, QPointF l1, QPointF l2, QPointF *closestLinePoint, double *pointRatio)
{
    auto M = l2 - l1;
    auto t0 = QPointF::dotProduct(M, point - l1) / QPointF::dotProduct(M, M);
    QPointF closestPoint;
    QVector2D orthVect;
    if (t0 <= 0) {
        orthVect = QVector2D(point - l1);
        closestPoint = l1;
        t0 = 0;
    } else if(t0 >= 1) {
        orthVect = QVector2D(point - l2);
        closestPoint = l2;
        t0 = 1;
    } else {
        auto intersect = l1 + t0 * M;
        orthVect = QVector2D(point - intersect);
        closestPoint = intersect;
    }
    if(closestLinePoint) {
        *closestLinePoint = closestPoint;
    }
    if(pointRatio) {
        *pointRatio = t0;
    }
    return orthVect.length();
}
