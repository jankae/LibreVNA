#ifndef SCPI_H
#define SCPI_H

#include <QString>
#include <QObject>
#include <vector>
#include <functional>

class SCPICommand {
public:
    SCPICommand(QString name, std::function<QString(QStringList)> cmd, std::function<QString(QStringList)> query) :
        _name(name),
        fn_cmd(cmd),
        fn_query(query){}

    QString execute(QStringList params);
    QString query(QStringList params);
    QString name() {return _name;}
    bool queryable() { return fn_query != nullptr;};
    bool executable() { return fn_cmd != nullptr;};
private:
    const QString _name;
    std::function<QString(QStringList)> fn_cmd;
    std::function<QString(QStringList)> fn_query;
};

class SCPINode {
    friend class SCPI;
public:
    SCPINode(QString name) :
        name(name){}

    bool add(SCPINode *node);
    bool add(SCPICommand *cmd);

private:
    QString parse(QString cmd, SCPINode* &lastNode);
    bool nameCollision(QString name);
    void createCommandList(QString prefix, QString &list);
    const QString name;
    std::vector<SCPINode*> subnodes;
    std::vector<SCPICommand*> commands;
};

class SCPI : public QObject, public SCPINode
{
    Q_OBJECT
public:
    SCPI();

    static bool match(QString s1, QString s2);
    static QString alternateName(QString name);

    static bool paramToDouble(QStringList params, int index, double &dest);
    static bool paramToULong(QStringList params, int index, unsigned long &dest);
    static bool paramToLong(QStringList params, int index, long &dest);

public slots:
    void input(QString line);
signals:
    void output(QString line);

private:
    SCPINode *lastNode;
};

#endif // SCPI_H
