#include "tracewidget.h"

#include "ui_tracewidget.h"
#include "traceeditdialog.h"
#include "traceimportdialog.h"
#include "ui_s2pImportOptions.h"
#include "CustomWidgets/informationbox.h"
#include "tracetouchstoneexport.h"
#include "trace.h"
#include "unit.h"
#include "Util/util.h"
#include "appwindow.h"

#include <QKeyEvent>
#include <QFileDialog>
#include <QDrag>
#include <QMimeData>
#include <QDebug>
#include <QMenu>
#include <QTableView>
#include <QDebug>

TraceWidget::TraceWidget(TraceModel &model, Calibration *cal, Deembedding *deembed, QWidget *parent) :
    QWidget(parent),
    SCPINode("TRACe"),
    dragTrace(nullptr),
    ui(new Ui::TraceWidget),
    model(model),
    dropFilename(""),
    cal(cal),
    deembed(deembed)
{
    ui->setupUi(this);
    ui->view->setModel(&model);
    ui->view->setAutoScroll(false);
    ui->view->viewport()->installEventFilter(this);
    connect(ui->bImport, &QPushButton::clicked, this, &TraceWidget::importDialog);
    connect(ui->bExport, &QPushButton::clicked, this, &TraceWidget::exportDialog);
    connect(ui->view->selectionModel(), &QItemSelectionModel::currentRowChanged, this, [=](const QModelIndex &current, const QModelIndex &previous){
        Q_UNUSED(previous)
        ui->edit->setEnabled(current.isValid());
        ui->remove->setEnabled(current.isValid());
    });
    installEventFilter(this);
    setAcceptDrops(true);
    createCount = 0;
    SetupSCPI();
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
            ui->edit->setEnabled(false);
            ui->remove->setEnabled(false);
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

void TraceWidget::dragEnterEvent(QDragEnterEvent *event)
{
    QString data = "";
    if(event->mimeData()->hasFormat("text/plain")) {
        data = QString(event->mimeData()->data("text/plain"));
        if (data.startsWith("file://")) {
            data = data.trimmed();
            data.remove(0, 7);
        } else {
            // something else
            data = "";
        }
    } else if(event->mimeData()->hasFormat("text/uri-list")) {
        data = QString(event->mimeData()->data("text/uri-list"));
        if (data.startsWith("file:///")) {
            data = data.trimmed();
            data.remove(0, 8);
        } else {
            // something else
            data = "";
        }
    }
    if(!data.isEmpty()) {
        // extract file path/name and type
        if(data.contains(".")) {
            auto type = data.split(".").last();
            if (supportsImportFileFormats().contains(type)) {
                dropFilename = data;
                qDebug() << "prepared to drop file " << dropFilename;
                event->accept();
                return;
            }
        }
    }
    event->ignore();
    dropFilename = "";
 }

void TraceWidget::dropEvent(QDropEvent *event)
{
    Q_UNUSED(event);
    if(dropFilename.size() > 0) {
        if(importFile(dropFilename)) {
            event->accept();
        }
    }
    dropFilename = "";
}

void TraceWidget::on_edit_clicked()
{
    if(ui->view->currentIndex().isValid()) {
        auto edit = new TraceEditDialog(*model.trace(ui->view->currentIndex().row()));
        if(AppWindow::showGUI()) {
            edit->show();
        }
    }
}

void TraceWidget::on_view_doubleClicked(const QModelIndex &index)
{
    if(index.column() == TraceModel::ColIndexName) {
        auto edit = new TraceEditDialog(*model.trace(index.row()));
        if(AppWindow::showGUI()) {
            edit->show();
        }
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
    case TraceModel::ColIndexDeembedding:
        model.toggleDeembedding(index.row());
        break;
    case TraceModel::ColIndexMath:
        model.toggleMath(index.row());
        break;
    default:
        break;
    }
}

void TraceWidget::importDialog()
{
    QString supported = "Supported files (";
    for(auto f : supportsImportFileFormats()) {
        supported += "*."+f+" ";
    }
    supported.chop(1);
    supported += ")";
    auto filename = QFileDialog::getOpenFileName(nullptr, "Open measurement file", "", supported, nullptr, Preferences::QFileDialogOptions());
    if (!filename.isEmpty()) {
        importFile(filename);
    }
}

bool TraceWidget::importFile(QString filename)
{
    if(!filename.contains(".")) {
        // no ending, not supported
        return false;
    }
    auto format = filename.split(".").last();
    if(!supportsImportFileFormats().contains(format)) {
        // unsupported format
        return false;
    }
    // try to import the file
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
                bool calAvailable = cal && cal->getNumPoints() > 0;
                bool deembedAvailable = deembed && deembed->getOptions().size() > 0;
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
                        cal->correctTraces(set);
                    }
                    if(applyDeembed) {
                        deembed->Deembed(set);
                    }
                }
            }
        });
        return true;
    } catch(const std::exception& e) {
        InformationBox::ShowError("Failed to import file", QString("Attempt to import file ended with error: \"") + e.what()+"\"");
        return false;
    }
}

void TraceWidget::SetupSCPI()
{
    auto findTraceFromName = [=](QString name) -> Trace* {
        // check if trace is specified by number
        bool ok;
        auto n = name.toUInt(&ok);
        if(ok) {
            // check if enough traces exist
            if(n < model.getTraces().size()) {
                return model.getTraces()[n];
            } else {
                // invalid number
                return nullptr;
            }
        } else {
            // trace specified by name
            for(auto t : model.getTraces()) {
                if(t->name().compare(name, Qt::CaseInsensitive) == 0) {
                    return t;
                }
            }
            // not found
            return nullptr;
        }
    };
    auto findTrace = [=](QStringList params) -> Trace* {
        if(params.size() < 1) {
            return nullptr;
        }
        return findTraceFromName(params[0]);
    };

    auto createStringFromData = [](Trace *t, const Trace::Data &d) -> QString {
        if(Trace::isSAParameter(t->liveParameter())) {
            if(std::isnan(d.x)) {
                return "NaN";
            }
            return QString::number(Util::SparamTodB(d.y.real()));
        } else {
            if(std::isnan(d.x)) {
                return "NaN,NaN";
            }
            return QString::number(d.y.real())+","+QString::number(d.y.imag());
        }
    };

    add(new SCPICommand("LIST", nullptr, [=](QStringList){
       QString ret;
       for(auto t : model.getTraces()) {
           ret += t->name() + ",";
       }
       ret.chop(1);
       return  ret;
    }));
    add(new SCPICommand("DATA", nullptr, [=](QStringList params) -> QString {
        auto t = findTrace(params);
        if(!t) {
           return SCPI::getResultName(SCPI::Result::Error);
        }
        QString ret;
        if(t->size() > 0) {
            for(unsigned int i=0;i<t->size();i++) {
                auto d = t->sample(i);
                int precision = 0;
                switch(t->outputType()) {
                case Trace::DataType::Invalid:
                case Trace::DataType::Frequency: precision = 0; break;
                case Trace::DataType::Time: precision = 12; break;
                case Trace::DataType::Power: precision = 3; break;
                case Trace::DataType::TimeZeroSpan: precision = 4; break;
                }
                ret += "[" + QString::number(d.x, 'f', precision) + ","+createStringFromData(t, d)+"],";
            }
            ret.chop(1);
        } else {
            ret = "EMPTY";
        }
        return ret;
    }));
    add(new SCPICommand("AT", nullptr, [=](QStringList params) -> QString {
        auto t = findTrace(params);
        if(!t) {
           return SCPI::getResultName(SCPI::Result::Error);
        }
        double x;
        if(!SCPI::paramToDouble(params, 1, x)) {
            return SCPI::getResultName(SCPI::Result::Error);
        } else {
            auto d = t->interpolatedSample(x);
            if(std::isnan(d.x)) {
                return "NaN,NaN";
            } else {
                return createStringFromData(t, d);
            }
        }
    }));
    add(new SCPICommand("TOUCHSTONE", nullptr, [=](QStringList params) -> QString {
        if(params.size() < 1) {
            // no traces given
            return SCPI::getResultName(SCPI::Result::Error);
        }
        // check number of paramaters, must be a square number
        int numTraces = params.size();
        int ports = round(sqrt(numTraces));
        if(ports * ports != numTraces) {
            // invalid number of traces
            return SCPI::getResultName(SCPI::Result::Error);
        }
        Trace* traces[numTraces];
        for(int i=0;i<numTraces;i++) {
            traces[i] = findTraceFromName(params[i]);
            if(!traces[i]) {
                // couldn't find that trace
                return SCPI::getResultName(SCPI::Result::Error);
            }
        }
        // check if trace selection is valid
        auto npoints = traces[0]->size();
        auto f_start = traces[0]->minX();
        auto f_stop = traces[0]->maxX();
        for(int i=0;i<ports;i++) {
            for(int j=0;j<ports;j++) {
                bool need_reflection = i==j;
                auto t = traces[j+i*ports];
                if(t->outputType() != Trace::DataType::Frequency) {
                    // invalid domain
                    return SCPI::getResultName(SCPI::Result::Error);
                }
                if(t->isReflection() != need_reflection) {
                    // invalid measurement at this position
                    return SCPI::getResultName(SCPI::Result::Error);
                }
                if((t->size() != npoints) || (t->minX() != f_start) || (t->maxX() != f_stop)) {
                    // frequency points are not identical
                    return SCPI::getResultName(SCPI::Result::Error);
                }
            }
        }
        // all traces checked, they are valid.
        // Constructing touchstone
        Touchstone t = Touchstone(ports);
        for(unsigned int i=0;i<npoints;i++) {
            Touchstone::Datapoint d;
            d.frequency = traces[0]->getSample(i).x;
            for(auto trace : traces) {
                d.S.push_back(trace->getSample(i).y);
            }
            t.AddDatapoint(d);
        }
        // touchstone assembled, save to dummyfile
        auto s = t.toString(Touchstone::Scale::GHz, Touchstone::Format::RealImaginary);
        return QString::fromStdString(s.str());
    }));
    add(new SCPICommand("MAXFrequency", nullptr, [=](QStringList params) -> QString {
        auto t = findTrace(params);
        if(!t) {
           return SCPI::getResultName(SCPI::Result::Error);
        }
        return QString::number(t->maxX(), 'f', 0);
    }));
    add(new SCPICommand("MINFrequency", nullptr, [=](QStringList params) -> QString {
        auto t = findTrace(params);
        if(!t) {
           return SCPI::getResultName(SCPI::Result::Error);
        }
        return QString::number(t->minX(), 'f', 0);
    }));
    add(new SCPICommand("MAXAmplitude", nullptr, [=](QStringList params) -> QString {
        auto t = findTrace(params);
        if(!t) {
           return SCPI::getResultName(SCPI::Result::Error);
        }
        auto d = t->interpolatedSample(t->findExtremum(true));
        return QString::number(d.x, 'f', 0)+","+createStringFromData(t, d);
    }));
    add(new SCPICommand("MINAmplitude", nullptr, [=](QStringList params) -> QString {
        auto t = findTrace(params);
        if(!t) {
           return SCPI::getResultName(SCPI::Result::Error);
        }
        auto d = t->interpolatedSample(t->findExtremum(false));
        return QString::number(d.x, 'f', 0)+","+createStringFromData(t, d);
    }));
    add(new SCPICommand("NEW", [=](QStringList params) -> QString {
        if(params.size() != 1) {
            return SCPI::getResultName(SCPI::Result::Error);
        }
        createCount++;
        auto t = new Trace(params[0], Qt::darkYellow, defaultParameter());
        t->setColor(QColor::fromHsl((createCount * 50) % 360, 250, 128));
        model.addTrace(t);
        return SCPI::getResultName(SCPI::Result::Empty);
    }, nullptr));
    add(new SCPICommand("DELete", [=](QStringList params) -> QString {
        auto t = findTrace(params);
        if(!t) {
           return SCPI::getResultName(SCPI::Result::Error);
        }
        model.removeTrace(t);
        return SCPI::getResultName(SCPI::Result::Empty);
    }, nullptr));
    add(new SCPICommand("RENAME", [=](QStringList params) -> QString {
        auto t = findTrace(params);
        if(!t) {
           return SCPI::getResultName(SCPI::Result::Error);
        }
        if(params.size() != 2) {
            return SCPI::getResultName(SCPI::Result::Error);
        }
        t->setName(params[1]);
        return SCPI::getResultName(SCPI::Result::Empty);
    }, nullptr));
    add(new SCPICommand("PAUSE", [=](QStringList params) -> QString {
        auto t = findTrace(params);
        if(!t) {
           return SCPI::getResultName(SCPI::Result::Error);
        }
        t->pause();
        return SCPI::getResultName(SCPI::Result::Empty);
    }, nullptr));
    add(new SCPICommand("RESUME", [=](QStringList params) -> QString {
        auto t = findTrace(params);
        if(!t) {
            return SCPI::getResultName(SCPI::Result::Error);
        }
        t->resume();
        return SCPI::getResultName(SCPI::Result::Empty);
    }, nullptr));
    add(new SCPICommand("PAUSED", nullptr, [=](QStringList params) -> QString {
        auto t = findTrace(params);
        if(!t) {
           return SCPI::getResultName(SCPI::Result::Error);
        }
        return t->isPaused() ? SCPI::getResultName(SCPI::Result::True) : SCPI::getResultName(SCPI::Result::False);
    }));
    auto deembed = new SCPINode("DEEMBedding");
    deembed->add(new SCPICommand("ACTive", [=](QStringList params) -> QString {
        Trace* t = findTrace(params);
        if(!t) {
           return SCPI::getResultName(SCPI::Result::Error);
        }
        bool activate = false;
        if(!SCPI::paramToBool(params, 1, activate)) {
            return SCPI::getResultName(SCPI::Result::Error);
        }
        if(activate) {
            if(!t->deembeddingAvailable()) {
                return SCPI::getResultName(SCPI::Result::Error);
            }
            t->setDeembeddingActive(true);
        } else {
            t->setDeembeddingActive(false);
        }
        return SCPI::getResultName(SCPI::Result::Empty);
    }, [=](QStringList params) -> QString {
        Trace* t = findTrace(params);
        if(!t) {
           return SCPI::getResultName(SCPI::Result::Error);
        }
        return t->isDeembeddingActive() ? SCPI::getResultName(SCPI::Result::True) : SCPI::getResultName(SCPI::Result::False);
    }));
    deembed->add(new SCPICommand("AVAILable", nullptr, [=](QStringList params) -> QString {
        Trace* t = findTrace(params);
        if(!t) {
           return SCPI::getResultName(SCPI::Result::Error);
        }
        return t->deembeddingAvailable() ? SCPI::getResultName(SCPI::Result::True) : SCPI::getResultName(SCPI::Result::False);
    }));
    add(deembed);
    add(new SCPICommand("PARAMeter", [=](QStringList params) -> QString {
        auto t = findTrace(params);
        if(!t || params.size() < 2) {
           return SCPI::getResultName(SCPI::Result::Error);
        }
        auto newparam = params[1];
        if((Trace::isVNAParameter(t->liveParameter()) && Trace::isVNAParameter(newparam))
                || (Trace::isSAParameter(t->liveParameter()) && Trace::isSAParameter(newparam))) {
            t->fromLivedata(t->liveType(), newparam);
            return SCPI::getResultName(SCPI::Result::Empty);
        } else {
            return SCPI::getResultName(SCPI::Result::Error);
        }
    }, [=](QStringList params) -> QString {
        auto t = findTrace(params);
        if(!t) {
           return SCPI::getResultName(SCPI::Result::Error);
        }
        return t->liveParameter();
    }));
    add(new SCPICommand("TYPE", [=](QStringList params) -> QString {
        auto t = findTrace(params);
        if(!t || params.size() < 2) {
           return SCPI::getResultName(SCPI::Result::Error);
        }
        auto newtype = Trace::TypeFromString(params[1]);
        if(newtype != Trace::LivedataType::Invalid) {
            t->fromLivedata(newtype, t->liveParameter());
            return SCPI::getResultName(SCPI::Result::Empty);
        } else {
            return SCPI::getResultName(SCPI::Result::Error);
        }
    }, [=](QStringList params) -> QString {
        auto t = findTrace(params);
        if(!t) {
           return SCPI::getResultName(SCPI::Result::Error);
        }
        return Trace::TypeToString(t->liveType());
    }));
}

void TraceWidget::contextMenuEvent(QContextMenuEvent *event)
{
    auto index = ui->view->indexAt(event->pos());
    if(!index.isValid()) {
        return;
    }
    auto trace = model.trace(index.row());
    auto ctxmenu = new QMenu();
    auto action_delete = new QAction("Delete");
    connect(action_delete, &QAction::triggered, this, &TraceWidget::on_remove_clicked);
    ctxmenu->addAction(action_delete);
    auto action_duplicate = new QAction("Duplicate");
    connect(action_duplicate, &QAction::triggered, this, [=](){
        auto json = trace->toJSON();
        auto duplicate = new Trace();
        duplicate->fromJSON(json);
        duplicate->setName(duplicate->name() + " - Duplicate");
        // force update of hash
        duplicate->toHash(true);
        model.addTrace(duplicate);
        // resolve math sources
        if(!duplicate->resolveMathSourceHashes()) {
            qWarning() << "Failed to resolve all math source hashes for"<<duplicate;
        }
    });
    ctxmenu->addAction(action_duplicate);
    ctxmenu->exec(event->globalPos());
}
