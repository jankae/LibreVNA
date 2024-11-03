#include "tracewidgetvna.h"

#include "Traces/traceimportdialog.h"
#include "Traces/tracetouchstoneexport.h"
#include "Traces/tracecsvexport.h"
#include "ui_tracewidget.h"
#include "ui_s2pImportOptions.h"
#include "CustomWidgets/informationbox.h"
#include "appwindow.h"

#include <QFileDialog>
#include <QMenu>

TraceWidgetVNA::TraceWidgetVNA(TraceModel &model, Calibration &cal, Deembedding &deembed, QWidget *parent)
    : TraceWidget(model, parent),
      cal(cal),
      deembed(deembed)
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

void TraceWidgetVNA::importDialog()
{
    auto filename = QFileDialog::getOpenFileName(nullptr, "Open measurement file", "", "Touchstone files (*.s1p *.s2p *.s3p *.s4p);;CSV files (*.csv)", nullptr, Preferences::QFileDialogOptions());
    if (!filename.isEmpty()) {
        try {
            std::vector<Trace*> traces;
            int touchstonePorts = 0;
            QString prefix = QString();
            if(filename.endsWith(".csv")) {
                auto csv = CSV::fromFile(filename);
                traces = Trace::createFromCSV(csv);
            } else {
                // must be a touchstone file
                auto t = Touchstone::fromFile(filename.toStdString());
                traces = Trace::createFromTouchstone(t);
                touchstonePorts = t.ports();
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
            if(AppWindow::showGUI()) {
                i->show();
            }
            // potential candidate to process via calibration/de-embedding
            connect(i, &TraceImportDialog::importFinsished, [=](const std::vector<Trace*> &traces) {
                if(traces.size() == touchstonePorts*touchstonePorts) {
                    // all traces imported, can calculate calibration/de-embedding
                    bool calAvailable = cal.getNumPoints() > 0;
                    bool deembedAvailable = deembed.getOptions().size() > 0;
                    if(calAvailable || deembedAvailable) {
                        // check if user wants to apply either one to the imported traces
                        auto dialog = new QDialog();
                        auto ui = new Ui::s2pImportOptions;
                        ui->setupUi(dialog);
                        connect(dialog, &QDialog::finished, [=](){
                            delete ui;
                        });
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
                        if(AppWindow::showGUI()) {
                            dialog->exec();
                        }
                        // assemble trace set
                        std::map<QString, Trace*> set;
                        for(int i=1;i<=touchstonePorts;i++) {
                            for(int j=1;j<=touchstonePorts;j++) {
                                QString name = "S"+QString::number(i)+QString::number(j);
                                int index = (i-1)*touchstonePorts+(j-1);
                                set[name] = traces[index];
                            }
                        }
                        if(applyCal) {
                                cal.correctTraces(set);
                        }
                        if(applyDeembed) {
                            deembed.Deembed(set);
                        }
                    }
                }
            });
        } catch(const std::exception& e) {
            InformationBox::ShowError("Failed to import file", QString("Attempt to import file ended with error: \"") + e.what()+"\"");
        }
    }
}
