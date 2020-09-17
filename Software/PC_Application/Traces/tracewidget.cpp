#include "tracewidget.h"
#include "ui_tracewidget.h"
#include "trace.h"
#include <QKeyEvent>
#include "traceeditdialog.h"
#include "traceimportdialog.h"
#include "traceexportdialog.h"
#include <QFileDialog>

TraceWidget::TraceWidget(TraceModel &model, QWidget *parent, bool SA) :
    QWidget(parent),
    ui(new Ui::TraceWidget),
    model(model),
    SA(SA)
{
    ui->setupUi(this);
    ui->view->setModel(&model);
    ui->view->setAutoScroll(false);
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
    auto liveParam = SA ? Trace::LiveParameter::Port1 : Trace::LiveParameter::S11;
    auto t = new Trace("Trace #"+QString::number(createCount), Qt::darkYellow, liveParam);
    t->setColor(QColor::fromHsl((createCount * 50) % 360, 250, 128));
    model.addTrace(t);
}

void TraceWidget::on_remove_clicked()
{
    model.removeTrace(ui->view->currentIndex().row());
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
    if(index.column() == 2) {
        auto edit = new TraceEditDialog(*model.trace(index.row()));
        edit->show();
    }
}

void TraceWidget::on_view_clicked(const QModelIndex &index)
{
    if(index.column()==0) {
        model.toggleVisibility(index.row());
    } else if(index.column()==1) {
        model.togglePause(index.row());
    }
}

void TraceWidget::on_bImport_clicked()
{
    auto filename = QFileDialog::getOpenFileName(nullptr, "Open measurement file", "", "Touchstone files (*.s1p *.s2p *.s3p *.s4p)", nullptr, QFileDialog::DontUseNativeDialog);
    if (filename.length() > 0) {
        auto t = Touchstone::fromFile(filename.toStdString());
        std::vector<Trace*> traces;
        for(unsigned int i=0;i<t.ports()*t.ports();i++) {
            auto trace = new Trace();
            trace->fillFromTouchstone(t, i, filename);
            unsigned int sink = i / t.ports() + 1;
            unsigned int source = i % t.ports() + 1;
            trace->setName("S"+QString::number(sink)+QString::number(source));
            traces.push_back(trace);
        }
        // contruct prefix from filename
        // remove any directory names (keep only the filename itself)
        int lastSlash = qMax(filename.lastIndexOf('/'), filename.lastIndexOf('\\'));
        if(lastSlash != -1) {
            filename.remove(0, lastSlash + 1);
        }
        // remove file type
        filename.truncate(filename.indexOf('.'));
        auto i = new TraceImportDialog(model, traces, filename+"_");
        i->show();
    }
}

void TraceWidget::on_bExport_clicked()
{
    auto e = new TraceExportDialog(model);
    // Attempt to set default traces (this will result in correctly populated
    // 2 port export if the initial 4 traces have not been modified)
    e->setPortNum(2);
    auto traces = model.getTraces();
    for(unsigned int i=0;i<4;i++) {
        if(i >= traces.size()) {
            break;
        }
        e->setTrace(i%2, i/2, traces[i]);
    }
    e->show();
}
