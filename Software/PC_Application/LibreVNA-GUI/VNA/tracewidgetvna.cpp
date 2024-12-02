#include "tracewidgetvna.h"

#include "Traces/traceimportdialog.h"
#include "Traces/tracetouchstoneexport.h"
#include "Traces/tracecsvexport.h"
#include "ui_tracewidget.h"
#include "appwindow.h"

#include <QFileDialog>
#include <QMenu>

TraceWidgetVNA::TraceWidgetVNA(TraceModel &model, Calibration *cal, Deembedding *deembed, QWidget *parent)
    : TraceWidget(model, cal, deembed, parent)
{
    auto exportMenu = new QMenu();
    auto exportTouchstoneAction = new QAction("Touchstone");
    auto exportCSVAction = new QAction("CSV");
    exportMenu->addAction(exportTouchstoneAction);
    exportMenu->addAction(exportCSVAction);

    ui->bExport->setMenu(exportMenu);

    connect(exportTouchstoneAction, &QAction::triggered, this, &TraceWidgetVNA::exportTouchstone);
    connect(exportCSVAction, &QAction::triggered, this, &TraceWidgetVNA::exportCSV);
}

void TraceWidgetVNA::exportCSV()
{
    auto e = new TraceCSVExport(model);
    if(AppWindow::showGUI()) {
        e->show();
    }
}

void TraceWidgetVNA::exportTouchstone()
{
    auto e = new TraceTouchstoneExport(model);
    // Attempt to set default traces (this will result in correctly populated
    // 2 port export if the initial 4 traces have not been modified)
    e->setPortNum(2);
    auto traces = model.getTraces();
    for(unsigned int i=0;i<4;i++) {
        if(i >= traces.size()) {
            break;
        }
        e->setTrace(i%2+1, i/2+1, traces[i]);
    }
    if(AppWindow::showGUI()) {
        e->show();
    }
}

