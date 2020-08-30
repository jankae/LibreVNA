/* -*- mode: C++ ; c-file-style: "stroustrup" -*- *****************************
 * Qwt Widget Library
 * Copyright (C) 1997   Josef Wilgen
 * Copyright (C) 2002   Uwe Rathmann
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the Qwt License, Version 1.0
 *****************************************************************************/

#include "qwtplotpiecewisecurve.h"

void QwtPlotPiecewiseCurve::drawCurve(QPainter *p, int style, const QwtScaleMap &xMap, const QwtScaleMap &yMap, const QRectF &canvasRect, int from, int to) const
{
    if (to < 0)
        to = dataSize() - 1;

    int first, last = from;
    while (last <= to)
    {
        first = last;
        while (first <= to && (isNaN(sample(first).y())))
            ++first;
        last = first;
        while (last <= to && !isNaN(sample(last).y()))
            ++last;
        if (first <= to)
            QwtPlotCurve::drawCurve(p, style, xMap, yMap, canvasRect, first, last - 1);
    }
}
