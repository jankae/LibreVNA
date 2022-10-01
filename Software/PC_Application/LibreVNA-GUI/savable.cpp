#include "savable.h"
#include "CustomWidgets/informationbox.h"

#include <QFileDialog>
#include <fstream>
#include <iomanip>

using namespace std;

bool Savable::openFromFileDialog(QString title, QString filetype)
{
    auto filename = QFileDialog::getOpenFileName(nullptr, title, "", filetype, nullptr, QFileDialog::DontUseNativeDialog);
    if(filename.isEmpty()) {
        // aborted selection
        return false;
    }
    ifstream file;
    file.open(filename.toStdString());
    if(!file.is_open()) {
        qWarning() << "Unable to open file:" << filename;
        return false;
    }
    nlohmann::json j;
    try {
        file >> j;
    } catch (exception &e) {
        InformationBox::ShowError("Error", "Failed to parse the setup file (" + QString(e.what()) + ")");
        qWarning() << "Parsing of setup file failed: " << e.what();
        file.close();
        return false;
    }
    file.close();
    fromJSON(j);
    return true;
}

bool Savable::saveToFileDialog(QString title, QString filetype, QString ending)
{
    auto filename = QFileDialog::getSaveFileName(nullptr, title, "", filetype, nullptr, QFileDialog::DontUseNativeDialog);
    if(filename.isEmpty()) {
        // aborted selection
        return false;
    }
    if(!ending.isEmpty()) {
        if(!filename.endsWith(ending)) {
            filename.append(ending);
        }
    }
    ofstream file;
    file.open(filename.toStdString());
    file << setw(4) << toJSON() << endl;
    file.close();
    return true;
}
