#include "tracewidgetsa.h"

#include "Traces/tracecsvexport.h"
#include "Traces/traceimportdialog.h"
#include "CustomWidgets/informationbox.h"
#include "appwindow.h"

#include <QFileDialog>

TraceWidgetSA::TraceWidgetSA(TraceModel &model, QWidget *parent)
    : TraceWidget(model, nullptr, nullptr, parent)
{

}

void TraceWidgetSA::exportDialog()
{
    auto csv = new TraceCSVExport(model);
    if(AppWindow::showGUI()) {
        csv->show();
    }
}
