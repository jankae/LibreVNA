#include "csv.h"
#include <exception>
#include <fstream>
#include <QStringList>
#include <iomanip>

using namespace std;

CSV::CSV()
{

}

CSV CSV::fromFile(QString filename, char sep)
{
    CSV csv;
    ifstream file;
    file.open(filename.toStdString());
    if(!file.is_open()) {
        throw runtime_error("Unable to open file:"+filename.toStdString());
    }
    string line;
    bool firstLine = true;
    while(getline(file, line)) {
        auto qline = QString::fromStdString(line);
        auto stringList = qline.split(sep);
        if(firstLine) {
            // create columns and set headers
            for(auto l : stringList) {
                if(l.isEmpty()) {
                    // header needs to be present, abort here
                    break;
                }
                Column c;
                c.header = l;
                csv._columns.push_back(c);
            }
            firstLine = false;
        } else {
            // not the header, attempt to parse data
            for(unsigned int i=0;i < csv._columns.size();i++) {
                double value = 0.0;
                if(i < (unsigned int) stringList.size()) {
                    value = stringList[i].toDouble();
                }
                csv._columns[i].data.push_back(value);
            }
        }
    }
    csv.filename = filename;
    return csv;
}

void CSV::toFile(QString filename, char sep)
{
    ofstream file;
    file.open(filename.toStdString());
    file << setprecision(10);
    unsigned maxlen = 0;
    for(auto c : _columns) {
        file << c.header.toStdString();
        file << sep;
        if(c.data.size() > maxlen) {
            maxlen = c.data.size();
        }
    }
    file << endl;
    for(unsigned int i=0;i<maxlen;i++) {
        for(auto c : _columns) {
            if(i < c.data.size()) {
                file << c.data[i] << sep;
            }
        }
        file << endl;
    }
    file.close();
    this->filename = filename;
}

std::vector<double> CSV::getColumn(QString header)
{
    for(auto c : _columns) {
        if(c.header == header) {
            return c.data;
        }
    }
    throw runtime_error("Header name not found");
}

std::vector<double> CSV::getColumn(unsigned int index)
{
    return _columns.at(index).data;
}

QString CSV::getHeader(unsigned int index)
{
    return _columns.at(index).header;
}

void CSV::addColumn(QString name, const std::vector<double> &data)
{
    Column c;
    c.header = name;
    c.data = data;
    _columns.push_back(c);
}

QString CSV::getFilename() const
{
    return filename;
}

void CSV::setFilename(const QString &value)
{
    filename = value;
}
