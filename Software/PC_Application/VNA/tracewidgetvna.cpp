#include "tracewidgetvna.h"

#include <QFileDialog>
#include "Traces/traceimportdialog.h"
#include "Traces/traceexportdialog.h"

TraceWidgetVNA::TraceWidgetVNA(TraceModel &model, QWidget *parent)
    : TraceWidget(model, parent)
{

}

void TraceWidgetVNA::exportDialog()
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

void TraceWidgetVNA::importDialog()
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
