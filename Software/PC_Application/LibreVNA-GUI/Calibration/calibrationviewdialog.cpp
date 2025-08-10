#include "calibrationviewdialog.h"
#include "ui_calibrationviewdialog.h"

#include <QGraphicsSimpleTextItem>
#include <QVector2D>

const QColor CalibrationViewDialog::colorNoCal = Qt::darkRed;
const QColor CalibrationViewDialog::colorHasCal = Qt::darkGreen;

CalibrationViewDialog::CalibrationViewDialog(Calibration *cal, unsigned int ports, QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::CalibrationViewDialog)
    , cal(cal)
    , ports(ports)
{
    setAttribute(Qt::WA_DeleteOnClose);
    ui->setupUi(this);
    ui->port->setMaximum(ports);
    scene = new QGraphicsScene();
    populateScene();
    ui->view->setScene(scene);

    connect(ui->port, &QSpinBox::valueChanged, this, &CalibrationViewDialog::populateScene);
}

CalibrationViewDialog::~CalibrationViewDialog()
{
    delete ui;
}

void CalibrationViewDialog::populateScene()
{
    scene->clear();
    auto colorF = QApplication::palette().text().color();
    auto colorB = QApplication::palette().base().color();
    auto pen = QPen(colorF);

    auto drawDot = [this](float x, float y, float size, QPen pen = QPen(), QBrush brush = QBrush()) {
        scene->addEllipse(x - size/2, y - size/2, size, size, pen, brush);
    };

    auto drawText = [this](float x, float y, QString s, QColor color, auto alignH = Qt::AlignCenter, auto alignV = Qt::AlignCenter, float rotation = 0.0f) {
        auto text = scene->addText(s);
        text->setRotation(rotation);
        switch(alignH) {
            default:
            case Qt::AlignLeft: break;
            case Qt::AlignCenter:
            case Qt::AlignHCenter: x -= text->boundingRect().bottomRight().x()/2; break;
            case Qt::AlignRight: x -= text->boundingRect().bottomRight().x(); break;
        }
        switch(alignV) {
            default:
            case Qt::AlignTop: break;
            case Qt::AlignCenter:
            case Qt::AlignVCenter: y -= text->boundingRect().bottomRight().y()/2; break;
            case Qt::AlignBottom: y -= text->boundingRect().bottomRight().y(); break;
        }
        text->setPos(QPointF(x, y));
        text->setDefaultTextColor(color);
    };

    auto drawPath = [this, drawText](QList<QPointF> vertices, QColor color, QString label = QString(), bool arrow = false) {
        // draw lines
        for(unsigned int i=1;i<vertices.size();i++) {

            scene->addLine(QLineF(vertices[i-1], vertices[i]), color);
            if(i == vertices.size() / 2) {
                // this is the middle segment, add label and arrow if required
                auto midPoint = (vertices[i-1]+vertices[i])/2;
                QVector2D direction = QVector2D(vertices[i] - vertices[i-1]);
                direction.normalize();
                auto ortho = QVector2D(-direction.y(), direction.x());
                if(arrow) {
                    auto poly = QPolygonF({
                        QPointF(midPoint + direction.toPointF()*arrowLength/2),
                        QPointF(midPoint - direction.toPointF()*arrowLength/2 + ortho.toPointF()*arrowWidth/2),
                        QPointF(midPoint - direction.toPointF()*arrowLength/2 - ortho.toPointF()*arrowWidth/2)
                    });
                    scene->addPolygon(poly, color, color);
                }
                if(!label.isEmpty()) {
                    auto pos = midPoint;
                    if(label.startsWith("_")) {
                        label.remove(0, 1);
                        pos -= ortho.toPointF() * labelDistance;
                    } else {
                        pos += ortho.toPointF() * labelDistance;
                    }
                    auto alignH = abs(direction.x()) > abs(direction.y()) ? Qt::AlignCenter : Qt::AlignLeft;
                    auto alignV = abs(direction.y()) > abs(direction.x()) ? Qt::AlignCenter : Qt::AlignTop;
                    drawText(pos.x(), pos.y(), label, color, alignH, alignV);
                }
            }
        }
    };

    auto DUTwidth = 2 * ports * pathSpacing;
    auto DUTstartX = ports * pathSpacing + 2*boxSpacing;

    // set the overall scene size
    scene->setSceneRect(0, 0, marginLeft + DUTstartX + DUTwidth + marginRight, marginTop + ports * portHeight + marginBottom);

    // create the DUT
    // rectangle
    scene->addRect(marginLeft + DUTstartX, marginTop, DUTwidth, ports * portHeight, pen);
    drawText(marginLeft + DUTstartX + DUTwidth/2, marginTop, "DUT", colorF, Qt::AlignCenter, Qt::AlignTop);
    // ports
    for(unsigned int i=1;i<=ports;i++) {
        // input marker
        drawDot(marginLeft + DUTstartX, marginTop + i*portHeight - portHeight/2 + portForwardYOffset, portSize, colorF, colorB);
        // output marker
        drawDot(marginLeft + DUTstartX, marginTop + i*portHeight - portHeight/2 + portReverseYOffset, portSize, colorF, colorB);
    }
    // the reflection path
    drawPath({
        QPointF(marginLeft + DUTstartX + portSize/2, marginTop + ui->port->value()*portHeight - portHeight/2 + portForwardYOffset),
        QPointF(marginLeft + DUTstartX + pathSpacing, marginTop + ui->port->value()*portHeight - portHeight/2 + portForwardYOffset),
        QPointF(marginLeft + DUTstartX + pathSpacing, marginTop + ui->port->value()*portHeight - portHeight/2 + portReverseYOffset),
        QPointF(marginLeft + DUTstartX + portSize/2, marginTop + ui->port->value()*portHeight - portHeight/2 + portReverseYOffset),
    }, colorF, "_S"+QString::number(ui->port->value())+QString::number(ui->port->value()), true);
    // the forward transmission paths
    float xstart = marginLeft + DUTstartX + pathSpacing;
    for(unsigned int i=1;i<=ports;i++) {
        if((int) i == ui->port->value()) {
            // skip, this is the reflection path
            continue;
        }
        drawDot(xstart, marginTop + ui->port->value()*portHeight - portHeight/2 + portForwardYOffset, junctionSize, colorF, colorF);
        drawPath({
            QPointF(xstart, marginTop + ui->port->value()*portHeight - portHeight/2 + portForwardYOffset),
            QPointF(xstart + pathSpacing, marginTop + ui->port->value()*portHeight - portHeight/2 + portForwardYOffset),
            QPointF(xstart + pathSpacing, marginTop + i*portHeight - portHeight/2 + portReverseYOffset),
            QPointF(marginLeft + DUTstartX + portSize/2, marginTop + i*portHeight - portHeight/2 + portReverseYOffset),
        }, colorF, QString((int) i > ui->port->value() ? "_" : "") + "S"+QString::number(i)+QString::number(ui->port->value()), true);
        xstart += pathSpacing;
    }
    // the reverse transmission paths
    bool first = true;
    for(unsigned int i=1;i<=ports;i++) {
        if((int) i == ui->port->value()) {
            // skip, this is the reflection path
            continue;
        }
        if(first) {
            first = false;
            drawDot(marginLeft + DUTstartX + pathSpacing, marginTop + ui->port->value()*portHeight - portHeight/2 + portReverseYOffset, junctionSize, colorF, colorF);
            drawPath({
                QPointF(marginLeft + DUTstartX + pathSpacing, marginTop + ui->port->value()*portHeight - portHeight/2 + portReverseYOffset),
                QPointF(xstart, marginTop + ui->port->value()*portHeight - portHeight/2 + portReverseYOffset),
            }, colorF, "", false);
        } else {
            drawDot(xstart, marginTop + ui->port->value()*portHeight - portHeight/2 + portReverseYOffset, junctionSize, colorF, colorF);
        }
        drawPath({
            QPointF(marginLeft + DUTstartX + portSize/2, marginTop + i*portHeight - portHeight/2 + portForwardYOffset),
            QPointF(xstart + pathSpacing, marginTop + i*portHeight - portHeight/2 + portForwardYOffset),
            QPointF(xstart + pathSpacing, marginTop + ui->port->value()*portHeight - portHeight/2 + portReverseYOffset),
            QPointF(xstart, marginTop + ui->port->value()*portHeight - portHeight/2 + portReverseYOffset),
        }, colorF, QString((int) i > ui->port->value() ? "" : "_") + "S"+QString::number(ui->port->value())+QString::number(i), true);
        xstart += pathSpacing;
    }

    // isolation, transmission and receiver match paths
    xstart = marginLeft;
    for(unsigned int i=1;i<=ports;i++) {
        if((int) i == ui->port->value()) {
            // skip, this is the reflection path
            continue;
        }
        // isolation
        drawDot(xstart, marginTop + ui->port->value()*portHeight - portHeight/2 + portForwardYOffset, junctionSize, colorF, colorF);
        drawPath({
            QPointF(xstart, marginTop + ui->port->value()*portHeight - portHeight/2 + portForwardYOffset),
            QPointF(xstart + pathSpacing, marginTop + ui->port->value()*portHeight - portHeight/2 + portForwardYOffset),
        }, colorF);
        drawPath({
            QPointF(xstart + pathSpacing, marginTop + ui->port->value()*portHeight - portHeight/2 + portForwardYOffset),
            QPointF(xstart + pathSpacing, marginTop + i*portHeight - portHeight/2 + portReverseYOffset),
        }, cal->hasIsolation(ui->port->value(), i) ? colorHasCal : colorNoCal, QString((int) i > ui->port->value() ? "_" : "") + "I"+QString::number(i)+QString::number(ui->port->value()), true);
        drawPath({
            QPointF(xstart + pathSpacing, marginTop + i*portHeight - portHeight/2 + portReverseYOffset),
            QPointF(marginLeft, marginTop + i*portHeight - portHeight/2 + portReverseYOffset),
        }, colorF);
        xstart += pathSpacing;
        // transmission
        drawDot(xstart, marginTop + i*portHeight - portHeight/2 + portReverseYOffset, junctionSize, colorF, colorF);
        drawPath({
            QPointF(marginLeft + DUTstartX - portSize / 2, marginTop + i*portHeight - portHeight/2 + portReverseYOffset),
            QPointF(marginLeft + DUTstartX - pathSpacing, marginTop + i*portHeight - portHeight/2 + portReverseYOffset),
        }, colorF);
        drawPath({
            QPointF(marginLeft + DUTstartX - pathSpacing, marginTop + i*portHeight - portHeight/2 + portReverseYOffset),
            QPointF(xstart, marginTop + i*portHeight - portHeight/2 + portReverseYOffset),
        }, cal->hasTransmissionTracking(ui->port->value(), i) ? colorHasCal : colorNoCal, "_T"+QString::number(i)+QString::number(ui->port->value()), true);
        // Reveicer match
        drawDot(marginLeft + DUTstartX - pathSpacing, marginTop + i*portHeight - portHeight/2 + portReverseYOffset, junctionSize, colorF, colorF);
        drawPath({
            QPointF(marginLeft + DUTstartX - pathSpacing, marginTop + i*portHeight - portHeight/2 + portReverseYOffset),
            QPointF(marginLeft + DUTstartX - pathSpacing, marginTop + i*portHeight - portHeight/2 + portForwardYOffset),
            QPointF(marginLeft + DUTstartX - portSize/2, marginTop + i*portHeight - portHeight/2 + portForwardYOffset),
        }, cal->hasReceiverMatch(ui->port->value(), i) ? colorHasCal : colorNoCal, "L"+QString::number(i)+QString::number(ui->port->value()), true);
    }

    // reflection error box
    drawDot(xstart, marginTop + ui->port->value()*portHeight - portHeight/2 + portForwardYOffset, junctionSize, colorF, colorF);
    drawDot(xstart + pathSpacing, marginTop + ui->port->value()*portHeight - portHeight/2 + portForwardYOffset, junctionSize, colorF, colorF);
    drawDot(xstart + pathSpacing, marginTop + ui->port->value()*portHeight - portHeight/2 + portReverseYOffset, junctionSize, colorF, colorF);
    drawDot(xstart + pathSpacing + boxSpacing, marginTop + ui->port->value()*portHeight - portHeight/2 + portForwardYOffset, junctionSize, colorF, colorF);
    drawDot(xstart + pathSpacing + boxSpacing, marginTop + ui->port->value()*portHeight - portHeight/2 + portReverseYOffset, junctionSize, colorF, colorF);
    // unity paths
    drawPath({
        QPointF(xstart, marginTop + ui->port->value()*portHeight - portHeight/2 + portForwardYOffset),
        QPointF(xstart + pathSpacing, marginTop + ui->port->value()*portHeight - portHeight/2 + portForwardYOffset),
        QPointF(xstart + pathSpacing + boxSpacing, marginTop + ui->port->value()*portHeight - portHeight/2 + portForwardYOffset),
        QPointF(marginLeft + DUTstartX - portSize/2, marginTop + ui->port->value()*portHeight - portHeight/2 + portForwardYOffset),
    }, colorF, "", true);
    drawPath({
        QPointF(marginLeft + DUTstartX - portSize/2, marginTop + ui->port->value()*portHeight - portHeight/2 + portReverseYOffset),
        QPointF(xstart + pathSpacing + boxSpacing, marginTop + ui->port->value()*portHeight - portHeight/2 + portReverseYOffset),
    }, colorF);
    drawPath({
        QPointF(marginLeft + portSize/2, marginTop + ui->port->value()*portHeight - portHeight/2 + portReverseYOffset),
        QPointF(xstart + pathSpacing, marginTop + ui->port->value()*portHeight - portHeight/2 + portReverseYOffset),
    }, colorF);

    // directivity
    drawPath({
        QPointF(xstart + pathSpacing, marginTop + ui->port->value()*portHeight - portHeight/2 + portForwardYOffset),
        QPointF(xstart + pathSpacing, marginTop + ui->port->value()*portHeight - portHeight/2 + portReverseYOffset),
    }, cal->hasDirectivity(ui->port->value()) ? colorHasCal : colorNoCal, "_D"+QString::number(ui->port->value()), true);

    // reflection tracking
    drawPath({
        QPointF(xstart + pathSpacing + boxSpacing, marginTop + ui->port->value()*portHeight - portHeight/2 + portReverseYOffset),
        QPointF(xstart + pathSpacing, marginTop + ui->port->value()*portHeight - portHeight/2 + portReverseYOffset),
    }, cal->hasReflectionTracking(ui->port->value()) ? colorHasCal : colorNoCal, "_R"+QString::number(ui->port->value()), true);

    // source match
    drawPath({
        QPointF(xstart + pathSpacing + boxSpacing, marginTop + ui->port->value()*portHeight - portHeight/2 + portReverseYOffset),
        QPointF(xstart + pathSpacing + boxSpacing, marginTop + ui->port->value()*portHeight - portHeight/2 + portForwardYOffset),
    }, cal->hasSourceMatch(ui->port->value()) ? colorHasCal : colorNoCal, "S"+QString::number(ui->port->value()), true);

    // create the VNA ports
    for(unsigned int i=1;i<=ports;i++) {
        // stimulus port
        if(i == (unsigned int) ui->port->value()) {
            drawDot(marginLeft, marginTop + i*portHeight - portHeight/2 + portForwardYOffset, portSize, colorF, colorB);
            drawText(marginLeft - portSize/2, marginTop + i*portHeight - portHeight/2 + portForwardYOffset, "a"+QString::number(i), colorF, Qt::AlignRight, Qt::AlignCenter);
        }
        // output marker
        drawDot(marginLeft, marginTop + i*portHeight - portHeight/2 + portReverseYOffset, portSize, colorF, colorB);
        drawText(marginLeft - portSize/2, marginTop + i*portHeight - portHeight/2 + portReverseYOffset, "b"+QString::number(i), colorF, Qt::AlignRight, Qt::AlignCenter);
    }

    // create the ports of the VNA

}
