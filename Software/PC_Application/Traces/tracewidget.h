#ifndef TRACEWIDGET_H
#define TRACEWIDGET_H

#include <QWidget>
#include "tracemodel.h"

namespace Ui {
class TraceWidget;
}

class TraceWidget : public QWidget
{
    Q_OBJECT

public:
    explicit TraceWidget(TraceModel &model, QWidget *parent = nullptr, bool SA = false);
    ~TraceWidget();

public slots:
    void on_add_clicked();

private slots:
    void on_remove_clicked();
    void on_edit_clicked();

    void on_view_doubleClicked(const QModelIndex &index);

    void on_view_clicked(const QModelIndex &index);

    void on_bImport_clicked();

    void on_bExport_clicked();

private:
    bool eventFilter(QObject *obj, QEvent *event) override;
    Ui::TraceWidget *ui;
    TraceModel &model;
    int createCount;
    bool SA;
};

#endif // TRACEWIDGET_H
