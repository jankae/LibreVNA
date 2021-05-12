#include "tracewidgetsa.h"
#include "Traces/tracecsvexport.h"
#include <QFileDialog>
#include "Traces/traceimportdialog.h"
#include "CustomWidgets/informationbox.h"

TraceWidgetSA::TraceWidgetSA(TraceModel &model, QWidget *parent)
    : TraceWidget(model, parent)
{

}

void TraceWidgetSA::exportDialog()
{
    auto csv = new TraceCSVExport(model);
    csv->show();
}

void TraceWidgetSA::importDialog()
{
    auto filename = QFileDialog::getOpenFileName(nullptr, "Open measurement file", "", "CSV files (*.csv)", nullptr, QFileDialog::DontUseNativeDialog);
    if (!filename.isEmpty()) {
        try {
            std::vector<Trace*> traces;
            QString prefix = QString();
            auto csv = CSV::fromFile(filename);
            traces = Trace::createFromCSV(csv);
            // contruct prefix from filename
            prefix = filename;
            // remove any directory names (keep only the filename itself)
            int lastSlash = qMax(prefix.lastIndexOf('/'), prefix.lastIndexOf('\\'));
            if(lastSlash != -1) {
                prefix.remove(0, lastSlash + 1);
            }
            // remove file type
            prefix.truncate(prefix.indexOf('.'));
            prefix.append("_");
            auto i = new TraceImportDialog(model, traces, prefix);
            i->show();
        } catch(const std::exception e) {
            InformationBox::ShowError("Failed to import file", QString("Attempt to import file ended with error: \"") + e.what()+"\"");
        }
    }
}
