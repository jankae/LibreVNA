#ifndef MARKERWIDGET_H
#define MARKERWIDGET_H

#include <QWidget>
#include "tracemarkermodel.h"

namespace Ui {
class MarkerWidget;
}

class MarkerWidget : public QWidget
{
    Q_OBJECT

public:
    explicit MarkerWidget(TraceMarkerModel &model, QWidget *parent = nullptr);
    ~MarkerWidget();

private slots:
    void on_bDelete_clicked();
    void on_bAdd_clicked();
    void updatePersistentEditors();

private:
    Ui::MarkerWidget *ui;
    TraceMarkerModel &model;
};

#endif // MARKERWIDGET_H
