#ifndef TRACEWIDGET_H
#define TRACEWIDGET_H

#include "tracemodel.h"
#include "scpi.h"
#include "Calibration/calibration.h"
#include "VNA/Deembedding/deembedding.h"

#include <QWidget>

namespace Ui {
class TraceWidget;
}

class TraceWidget : public QWidget, public SCPINode
{
    Q_OBJECT

public:
    explicit TraceWidget(TraceModel &model, Calibration *cal, Deembedding *deembed, QWidget *parent = nullptr);
    virtual ~TraceWidget();

public:
    virtual void exportDialog() = 0;
    virtual void importDialog();

protected slots:
    void on_add_clicked();
    void on_remove_clicked();
    void on_edit_clicked();
    void on_view_doubleClicked(const QModelIndex &index);
    void on_view_clicked(const QModelIndex &index);
    virtual QStringList supportsImportFileFormats() = 0;
    bool importFile(QString filename);

protected:
    void SetupSCPI();
    void contextMenuEvent(QContextMenuEvent *event) override;
    bool eventFilter(QObject *obj, QEvent *event) override;
    void dragEnterEvent(QDragEnterEvent *event) override;
    void dropEvent(QDropEvent *event) override;
    virtual QString defaultParameter() = 0;
    QPoint dragStartPosition;
    Trace *dragTrace;
    Ui::TraceWidget *ui;
    TraceModel &model;
    int createCount;

private:
    QString dropFilename;
    // These can optionally be applied when importing an s2p file
    Calibration *cal;
    Deembedding *deembed;
};

#endif // TRACEWIDGET_H
