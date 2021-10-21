#include "tracewidget.h"

#include "ui_tracewidget.h"
#include "traceeditdialog.h"
#include "traceimportdialog.h"
#include "tracetouchstoneexport.h"
#include "trace.h"
#include "unit.h"
#include "Util/util.h"

#include <QKeyEvent>
#include <QFileDialog>
#include <QDrag>
#include <QMimeData>
#include <QDebug>
#include <QMenu>


TraceWidget::TraceWidget(TraceModel &model, QWidget *parent) :
    QWidget(parent),
    SCPINode("TRACe"),
    ui(new Ui::TraceWidget),
    model(model)
{
    ui->setupUi(this);
    ui->view->setModel(&model);
    ui->view->setAutoScroll(false);
    ui->view->viewport()->installEventFilter(this);
    connect(ui->bImport, &QPushButton::clicked, this, &TraceWidget::importDialog);
    connect(ui->bExport, &QPushButton::clicked, this, &TraceWidget::exportDialog);
    installEventFilter(this);
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

void TraceWidget::on_edit_clicked()
{
    if(ui->view->currentIndex().isValid()) {
        auto edit = new TraceEditDialog(*model.trace(ui->view->currentIndex().row()));
        edit->show();
    }
}

void TraceWidget::on_view_doubleClicked(const QModelIndex &index)
{
    if(index.column() == TraceModel::ColIndexName) {
        auto edit = new TraceEditDialog(*model.trace(index.row()));
        edit->show();
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
    case TraceModel::ColIndexMath:
        model.toggleMath(index.row());
        break;
    default:
        break;
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
        if(Trace::isSAParamater(t->liveParameter())) {
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
           return "ERROR";
        }
        QString ret;
        for(unsigned int i=0;i<t->size();i++) {
            auto d = t->sample(i);
            ret += "["+QString::number(d.x)+","+createStringFromData(t, d)+"],";
        }
        ret.chop(1);
        return ret;
    }));
    add(new SCPICommand("AT", nullptr, [=](QStringList params) -> QString {
        auto t = findTrace(params);
        if(!t) {
           return "ERROR";
        }
        double x;
        if(!SCPI::paramToDouble(params, 1, x)) {
            return "ERROR";
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
            return "ERROR";
        }
        // check number of paramaters, must be a square number
        int numTraces = params.size();
        int ports = round(sqrt(numTraces));
        if(ports * ports != numTraces) {
            // invalid number of traces
            return "ERROR";
        }
        Trace* traces[numTraces];
        for(int i=0;i<numTraces;i++) {
            traces[i] = findTraceFromName(params[i]);
            if(!traces[i]) {
                // couldn't find that trace
                return "ERROR";
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
                if(t->getDataType() != Trace::DataType::Frequency) {
                    // invalid domain
                    return "ERROR";
                }
                if(t->isReflection() != need_reflection) {
                    // invalid measurement at this position
                    return "ERROR";
                }
                if((t->size() != npoints) || (t->minX() != f_start) || (t->maxX() != f_stop)) {
                    // frequency points are not identical
                    return "ERROR";
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
           return "ERROR";
        }
        return QString::number(t->maxX());
    }));
    add(new SCPICommand("MINFrequency", nullptr, [=](QStringList params) -> QString {
        auto t = findTrace(params);
        if(!t) {
           return "ERROR";
        }
        return QString::number(t->minX());
    }));
    add(new SCPICommand("MAXAmplitude", nullptr, [=](QStringList params) -> QString {
        auto t = findTrace(params);
        if(!t) {
           return "ERROR";
        }
        auto d = t->interpolatedSample(t->findExtremum(true));
        return QString::number(d.x)+","+createStringFromData(t, d);
    }));
    add(new SCPICommand("MINAmplitude", nullptr, [=](QStringList params) -> QString {
        auto t = findTrace(params);
        if(!t) {
           return "ERROR";
        }
        auto d = t->interpolatedSample(t->findExtremum(false));
        return QString::number(d.x)+","+createStringFromData(t, d);
    }));
    add(new SCPICommand("NEW", [=](QStringList params) -> QString {
        if(params.size() != 1) {
            return "ERROR";
        }
        createCount++;
        auto t = new Trace(params[0], Qt::darkYellow, defaultParameter());
        t->setColor(QColor::fromHsl((createCount * 50) % 360, 250, 128));
        model.addTrace(t);
        return "";
    }, nullptr));
    add(new SCPICommand("RENAME", [=](QStringList params) -> QString {
        auto t = findTrace(params);
        if(!t) {
           return "ERROR";
        }
        if(params.size() != 2) {
            return "ERROR";
        }
        t->setName(params[1]);
        return "";
    }, nullptr));
    add(new SCPICommand("PAUSE", [=](QStringList params) -> QString {
        auto t = findTrace(params);
        if(!t) {
           return "ERROR";
        }
        t->pause();
        return "";
    }, nullptr));
    add(new SCPICommand("RESUME", [=](QStringList params) -> QString {
        auto t = findTrace(params);
        if(!t) {
           return "ERROR";
        }
        t->resume();
        return "";
    }, nullptr));
    add(new SCPICommand("PAUSED", nullptr, [=](QStringList params) -> QString {
        auto t = findTrace(params);
        if(!t) {
           return "ERROR";
        }
        return t->isPaused() ? "TRUE" : "FALSE";
    }));
    add(new SCPICommand("PARAMeter", [=](QStringList params) -> QString {
        auto t = findTrace(params);
        if(!t || params.size() < 2) {
           return "ERROR";
        }
        auto newparam = Trace::ParameterFromString(params[1]);
        if((Trace::isVNAParameter(t->liveParameter()) && Trace::isVNAParameter(newparam))
                || (Trace::isVNAParameter(t->liveParameter()) && Trace::isVNAParameter(newparam))) {
            t->fromLivedata(t->liveType(), newparam);
            return "";
        } else {
            return "ERROR";
        }
    }, [=](QStringList params) -> QString {
        auto t = findTrace(params);
        if(!t) {
           return "ERROR";
        }
        return Trace::ParameterToString(t->liveParameter());
    }));
    add(new SCPICommand("TYPE", [=](QStringList params) -> QString {
        auto t = findTrace(params);
        if(!t || params.size() < 2) {
           return "ERROR";
        }
        auto newtype = Trace::TypeFromString(params[1]);
        if(newtype != Trace::LivedataType::Invalid) {
            t->fromLivedata(newtype, t->liveParameter());
            return "";
        } else {
            return "ERROR";
        }
    }, [=](QStringList params) -> QString {
        auto t = findTrace(params);
        if(!t) {
           return "ERROR";
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
        model.addTrace(duplicate);
    });
    ctxmenu->addAction(action_duplicate);
    ctxmenu->exec(event->globalPos());
}
