#ifndef CSV_H
#define CSV_H

#include <QString>
#include <vector>

class CSV
{
public:
    CSV();

    static CSV fromFile(QString filename, char sep = ',');

    void toFile(QString filename, char sep = ',');
    std::vector<double> getColumn(QString header);
    std::vector<double> getColumn(unsigned int index);
    QString getHeader(unsigned int index);
    unsigned int columns() { return _columns.size();}

    void addColumn(QString name, const std::vector<double> &data);

    QString getFilename() const;
    void setFilename(const QString &value);

private:
    class Column {
    public:
        QString header;
        std::vector<double> data;
    };
    std::vector<Column> _columns;
    QString filename;
};

#endif // CSV_H
