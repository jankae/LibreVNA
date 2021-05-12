#include "tracewidgetvna.h"

#include <QFileDialog>
#include "Traces/traceimportdialog.h"
#include "Traces/tracetouchstoneexport.h"
#include "Traces/tracecsvexport.h"
#include "ui_tracewidget.h"
#include "ui_s2pImportOptions.h"
#include "CustomWidgets/informationbox.h"

#include <QMenu>

TraceWidgetVNA::TraceWidgetVNA(TraceModel &model, Calibration &cal, Deembedding &deembed, QWidget *parent)
    : TraceWidget(model, parent),
      cal(cal),
      deembed(deembed)
{
    auto exportMenu = new QMenu();
    auto exportTouchstone = new QAction("Touchstone");
    auto exportCSV = new QAction("CSV");
    exportMenu->addAction(exportTouchstone);
    exportMenu->addAction(exportCSV);

    ui->bExport->setMenu(exportMenu);

    connect(exportTouchstone, &QAction::triggered, [&]() {
        auto e = new TraceTouchstoneExport(model);
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
    });

    connect(exportCSV, &QAction::triggered, [&]() {
        auto e = new TraceCSVExport(model);
        e->show();
    });
}

void TraceWidgetVNA::importDialog()
{
    auto filename = QFileDialog::getOpenFileName(nullptr, "Open measurement file", "", "Touchstone files (*.s1p *.s2p *.s3p *.s4p);;CSV files (*.csv)", nullptr, QFileDialog::DontUseNativeDialog);
    if (!filename.isEmpty()) {
        try {
            std::vector<Trace*> traces;
            QString prefix = QString();
            if(filename.endsWith(".csv")) {
                auto csv = CSV::fromFile(filename);
                traces = Trace::createFromCSV(csv);
            } else {
                // must be a touchstone file
                auto t = Touchstone::fromFile(filename.toStdString());
                traces = Trace::createFromTouchstone(t);
            }
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
            if(filename.endsWith(".s2p")) {
                // potential candidate to process via calibration/de-embedding
                connect(i, &TraceImportDialog::importFinsished, [=](const std::vector<Trace*> &traces) {
                    if(traces.size() == 4) {
                        // all traces imported, can calculate calibration/de-embedding
                        bool calAvailable = cal.nPoints() > 0;
                        bool deembedAvailable = deembed.getOptions().size() > 0;
                        if(calAvailable || deembedAvailable) {
                            // check if user wants to apply either one to the imported traces
                            auto dialog = new QDialog();
                            auto ui = new Ui::s2pImportOptions;
                            ui->setupUi(dialog);
                            ui->applyCal->setEnabled(calAvailable);
                            ui->deembed->setEnabled(deembedAvailable);
                            bool applyCal = false;
                            bool applyDeembed = false;
                            connect(ui->applyCal, &QCheckBox::toggled, [&](bool checked) {
                                applyCal = checked;
                            });
                            connect(ui->deembed, &QCheckBox::toggled, [&](bool checked) {
                                applyDeembed = checked;
                            });
                            dialog->exec();
                            if(applyCal) {
                                cal.correctTraces(*traces[0], *traces[1], *traces[2], *traces[3]);
                            }
                            if(applyDeembed) {
                                deembed.Deembed(*traces[0], *traces[1], *traces[2], *traces[3]);
                            }
                        }
                    }
                });
            }
        } catch(const std::exception& e) {
            InformationBox::ShowError("Failed to import file", QString("Attempt to import file ended with error: \"") + e.what()+"\"");
        }
    }
}
