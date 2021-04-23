#ifndef TRACEWIDGET_H
#define TRACEWIDGET_H

#include <QWidget>
#include "tracemodel.h"
#include "scpi.h"

namespace Ui {
class TraceWidget;
}

class TraceWidget : public QWidget, public SCPINode
{
    Q_OBJECT

public:
    explicit TraceWidget(TraceModel &model, QWidget *parent = nullptr);
    ~TraceWidget();

protected slots:
    void on_add_clicked();
    void on_remove_clicked();
    void on_edit_clicked();
    void on_view_doubleClicked(const QModelIndex &index);
    void on_view_clicked(const QModelIndex &index);
    virtual void exportDialog() = 0;
    virtual void importDialog() = 0;

protected:
    void SetupSCPI();
    void contextMenuEvent(QContextMenuEvent *event) override;
    bool eventFilter(QObject *obj, QEvent *event) override;
    virtual Trace::LiveParameter defaultParameter() = 0;
    QPoint dragStartPosition;
    Trace *dragTrace;
    Ui::TraceWidget *ui;
    TraceModel &model;
    int createCount;
};

#endif // TRACEWIDGET_H
