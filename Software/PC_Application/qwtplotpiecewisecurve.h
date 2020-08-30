/* -*- mode: C++ ; c-file-style: "stroustrup" -*- *****************************
 * Qwt Widget Library
 * Copyright (C) 1997   Josef Wilgen
 * Copyright (C) 2002   Uwe Rathmann
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the Qwt License, Version 1.0
 *****************************************************************************/

#ifndef QWT_PLOT_PIECEWISE_CURVE_H
#define QWT_PLOT_PIECEWISE_CURVE_H

#include "qwt_plot_curve.h"

/*!
  \brief A class which draws piecewise curves

  This class can be used to display data with missing (NaN) values as a
  piecewise curve in the  x-y plane.
*/
class QWT_EXPORT QwtPlotPiecewiseCurve: public QwtPlotCurve
{
public:
    explicit QwtPlotPiecewiseCurve();
    explicit QwtPlotPiecewiseCurve(const QwtText &title);
    explicit QwtPlotPiecewiseCurve(const QString &title);
    ~QwtPlotPiecewiseCurve();

    virtual void drawCurve(QPainter *p, int style,
                           const QwtScaleMap &xMap, const QwtScaleMap &yMap,
                           const QRectF &canvasRect, int from, int to) const override;

private:
    static bool isNaN(double x);
};

inline QwtPlotPiecewiseCurve::QwtPlotPiecewiseCurve(): QwtPlotCurve()
{
}

inline QwtPlotPiecewiseCurve::QwtPlotPiecewiseCurve(const QwtText &title):
    QwtPlotCurve(title)
{
}

inline QwtPlotPiecewiseCurve::QwtPlotPiecewiseCurve(const QString &title):
    QwtPlotCurve(title)
{
}

inline QwtPlotPiecewiseCurve::~QwtPlotPiecewiseCurve()
{

}

inline bool QwtPlotPiecewiseCurve::isNaN(double x)
{
    return x != x;
}

#endif
