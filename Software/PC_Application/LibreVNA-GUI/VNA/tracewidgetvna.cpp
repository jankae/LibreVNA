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
    if(AppWindow::showGUI()) {
        e->show();
    }
}

