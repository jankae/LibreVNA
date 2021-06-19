#ifndef MARKERWIDGET_H
#define MARKERWIDGET_H

#include <QWidget>
#include "markermodel.h"

namespace Ui {
class MarkerWidget;
}

class MarkerWidget : public QWidget
{
    Q_OBJECT

public:
    explicit MarkerWidget(MarkerModel &model, QWidget *parent = nullptr);
    ~MarkerWidget();

private slots:
    void on_bDelete_clicked();
    void on_bAdd_clicked();
    void updatePersistentEditors();

private:
    bool eventFilter(QObject *obj, QEvent *event) override;
    Ui::MarkerWidget *ui;
    MarkerModel &model;
};

#endif // MARKERWIDGET_H
