#ifndef SCPI_H
#define SCPI_H

#include <QString>
#include <QObject>
#include <vector>
#include <functional>

class SCPICommand {
public:
    SCPICommand(QString name, std::function<QString(QStringList)> cmd, std::function<QString(QStringList)> query, bool convertToUppercase = true) :
        _name(name),
        fn_cmd(cmd),
        fn_query(query),
        argAlwaysUppercase(convertToUppercase){}

    QString execute(QStringList params);
    QString query(QStringList params);
    QString name() {return _name;}
    bool queryable() { return fn_query != nullptr;}
    bool executable() { return fn_cmd != nullptr;}
    bool convertToUppercase() { return argAlwaysUppercase;}
private:
    const QString _name;
    std::function<QString(QStringList)> fn_cmd;
    std::function<QString(QStringList)> fn_query;
    bool argAlwaysUppercase;
};

class SCPINode {
    friend class SCPI;
public:
    SCPINode(QString name) :
        name(name), parent(nullptr){}
    virtual ~SCPINode();

    bool add(SCPINode *node);
    bool remove(SCPINode *node);
    bool add(SCPICommand *cmd);

    bool addDoubleParameter(QString name, double &param, bool gettable = true, bool settable = true, std::function<void(void)> setCallback = nullptr);
    bool addUnsignedIntParameter(QString name, unsigned int &param, bool gettable = true, bool settable = true, std::function<void(void)> setCallback = nullptr);
    bool addBoolParameter(QString name, bool &param, bool gettable = true, bool settable = true, std::function<void(void)> setCallback = nullptr);

    bool changeName(QString newname);

private:
    QString parse(QString cmd, SCPINode* &lastNode);
    bool nameCollision(QString name);
    void createCommandList(QString prefix, QString &list);
    QString name;
    std::vector<SCPINode*> subnodes;
    std::vector<SCPICommand*> commands;
    SCPINode *parent;
};

class SCPI : public QObject, public SCPINode
{
    Q_OBJECT
public:
    SCPI();

    static bool match(QString s1, QString s2);
    static QString alternateName(QString name);

    static bool paramToDouble(QStringList params, int index, double &dest);
    static bool paramToULongLong(QStringList params, int index, unsigned long long &dest);
    static bool paramToLong(QStringList params, int index, long &dest);
    static bool paramToBool(QStringList params, int index, bool &dest);

    enum class Result {
        Empty,
        Error,
        False,
        True
    };

    static QString getResultName(SCPI::Result r);

public slots:
    void input(QString line);
signals:
    void output(QString line);
};

#endif // SCPI_H
