#include "tracewidgetsa.h"

#include "Traces/tracecsvexport.h"

TraceWidgetSA::TraceWidgetSA(TraceModel &model, QWidget *parent)
    : TraceWidget(model, parent)
{

}

void TraceWidgetSA::exportDialog()
{
    auto csv = new TraceCSVExport(model);
    csv->show();
}
