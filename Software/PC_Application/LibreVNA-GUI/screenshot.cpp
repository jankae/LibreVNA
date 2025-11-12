#include "screenshot.h"

#include "preferences.h"

#include <QCoreApplication>
#include <QPainter>
#include <QSvgGenerator>

void SaveScreenshot(QWidget *widget)
{
    Q_ASSERT(widget != nullptr);

    const QStringList extensions = QStringList() << "png" << "svg";
    QStringList filters;

    for (const QString& ext: extensions) {
        filters << QString("%1 image files (*.%2)").arg(ext.toUpper(), ext);
    }

    auto& settings = Preferences::getInstance().UISettings;
    const QString filterString = filters.join(";;");
    qsizetype filterIndex = qBound(0, settings.saveImageFilterIndex, filters.size() - 1);
    QString selectedFilter = filters[filterIndex];

    auto filename = QFileDialog::getSaveFileName(nullptr, "Save plot image", settings.Paths.image, filterString, &selectedFilter, Preferences::QFileDialogOptions());
    if(filename.isEmpty()) {
        // aborted selection
        return;
    }

    filterIndex = filters.indexOf(selectedFilter);
    const QString& extension = extensions[filterIndex];
    if(!filename.endsWith(extension)) {
        filename += '.' + extension;
    }

    settings.Paths.image = QFileInfo(filename).path();
    settings.saveImageFilterIndex = filterIndex;

    switch (filterIndex)
    {
    case 0:  // PNG
        widget->grab().save(filename);
        break;

    case 1:  // SVG
    {
        QSvgGenerator generator;
        generator.setFileName(filename);
        generator.setViewBox(QRect(QPoint(), widget->size()));
        generator.setTitle(QCoreApplication::applicationName());
        generator.setDescription(QString("Created by %1 %2").arg(QCoreApplication::applicationName(), QCoreApplication::applicationVersion()));

        QPainter painter;
        painter.begin(&generator);
        widget->render(&painter);
        painter.end();
        break;
    }

    default:
        Q_ASSERT(false);
        break;
    }
}
