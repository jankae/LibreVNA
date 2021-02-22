#include "tracewidget.h"
#include "ui_tracewidget.h"
#include "trace.h"
#include <QKeyEvent>
#include "traceeditdialog.h"
#include "traceimportdialog.h"
#include "tracetouchstoneexport.h"
#include <QFileDialog>
#include <QDrag>
#include <QMimeData>
#include <QDebug>

TraceWidget::TraceWidget(TraceModel &model, QWidget *parent) :
    QWidget(parent),
    ui(new Ui::TraceWidget),
    model(model)
{
    ui->setupUi(this);
    ui->view->setModel(&model);
    ui->view->setAutoScroll(false);
    ui->view->viewport()->installEventFilter(this);
    connect(ui->bImport, &QPushButton::clicked, this, &TraceWidget::importDialog);
    connect(ui->bExport, &QPushButton::clicked, this, &TraceWidget::exportDialog);
    installEventFilter(this);
    createCount = 0;
}

TraceWidget::~TraceWidget()
{
    delete ui;
}

void TraceWidget::on_add_clicked()
{
    createCount++;
    auto t = new Trace("Trace #"+QString::number(createCount), Qt::darkYellow, defaultParameter());
    t->setColor(QColor::fromHsl((createCount * 50) % 360, 250, 128));
    model.addTrace(t);
    ui->view->selectRow(model.getTraces().size() - 1);
    on_edit_clicked();
}

void TraceWidget::on_remove_clicked()
{
    QModelIndex index = ui->view->currentIndex();
    if (index.isValid()) {      // if nothing clicked, index.row() = -1
        model.removeTrace(index.row());
        // otherwise, TraceModel casts index to unsigned int and compares with traces.size() which is int
    };
}

bool TraceWidget::eventFilter(QObject *, QEvent *event)
{
    if (event->type() == QEvent::KeyPress) {
        int key = static_cast<QKeyEvent *>(event)->key();
        if(key == Qt::Key_Escape) {
            ui->view->clearSelection();
            return true;
        } else if(key == Qt::Key_Delete) {
            model.removeTrace(ui->view->currentIndex().row());
            return true;
        }
    } else if(event->type() == QEvent::MouseButtonPress) {
        auto mouseEvent = static_cast<QMouseEvent*>(event);
        if (mouseEvent->button() == Qt::LeftButton) {
            auto index = ui->view->indexAt(mouseEvent->pos());
            if(index.isValid()) {
                dragStartPosition = mouseEvent->pos();
                dragTrace = model.trace(index.row());
            } else {
                dragTrace = nullptr;
            }
        }
        return false;
    } else if(event->type() == QEvent::MouseMove) {
        auto mouseEvent = static_cast<QMouseEvent*>(event);
        if (!(mouseEvent->buttons() & Qt::LeftButton)) {
            return false;
        }
        if (!dragTrace) {
            return false;
        }
        if ((mouseEvent->pos() - dragStartPosition).manhattanLength()
             < QApplication::startDragDistance()) {
            return false;
        }

        QDrag *drag = new QDrag(this);
        QMimeData *mimeData = new QMimeData;

        QByteArray encodedPointer;
        QDataStream stream(&encodedPointer, QIODevice::WriteOnly);
        stream << quintptr(dragTrace);

        mimeData->setData("trace/pointer", encodedPointer);
        drag->setMimeData(mimeData);

        drag->exec(Qt::CopyAction);
        return true;
    }
    return false;
}

void TraceWidget::on_edit_clicked()
{
    if(ui->view->currentIndex().isValid()) {
        auto edit = new TraceEditDialog(*model.trace(ui->view->currentIndex().row()));
        edit->show();
    }
}

void TraceWidget::on_view_doubleClicked(const QModelIndex &index)
{
    if(index.column() == TraceModel::ColIndexName) {
        auto edit = new TraceEditDialog(*model.trace(index.row()));
        edit->show();
    }
}

void TraceWidget::on_view_clicked(const QModelIndex &index)
{
    switch(index.column()) {
    case TraceModel::ColIndexVisible:
        model.toggleVisibility(index.row());
        break;
    case TraceModel::ColIndexPlayPause:
        model.togglePause(index.row());
        break;
    case TraceModel::ColIndexMath:
        model.toggleMath(index.row());
        break;
    default:
        break;
    }
}
