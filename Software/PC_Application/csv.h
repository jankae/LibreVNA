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
    unsigned int columns() { return _columns.size();}

    void addColumn(QString name, const std::vector<double> &data);

private:
    class Column {
    public:
        QString header;
        std::vector<double> data;
    };
    std::vector<Column> _columns;
};

#endif // CSV_H
