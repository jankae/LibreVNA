#ifndef SCPI_H
#define SCPI_H

#include <QString>
#include <QObject>
#include <QSemaphore>
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
    QString leafName() {return _name.split(":").back();}
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
        name(name), parent(nullptr), operationPending(false){}
    virtual ~SCPINode();

    bool add(SCPINode *node) {return addInternal(node, 0);}
    bool remove(SCPINode *node) {return removeInternal(node, 0);}
    bool add(SCPICommand *cmd) {return addInternal(cmd, 0);}
    bool remove(SCPICommand *cmd) {return removeInternal(cmd, 0);}

    bool addDoubleParameter(QString name, double &param, bool gettable = true, bool settable = true, std::function<void(void)> setCallback = nullptr);
    bool addUnsignedIntParameter(QString name, unsigned int &param, bool gettable = true, bool settable = true, std::function<void(void)> setCallback = nullptr);
    bool addBoolParameter(QString name, bool &param, bool gettable = true, bool settable = true, std::function<void(void)> setCallback = nullptr);

    bool changeName(QString newname);
    QString leafName() {return name.split(":").back();}

    void setOperationPending(bool pending);

protected:
    bool isOperationPending();

private:
    QString parse(QString cmd, SCPINode* &lastNode);
    bool nameCollision(QString name);
    void createCommandList(QString prefix, QString &list);
    SCPINode *findSubnode(QString name);
    bool addInternal(SCPINode *node, int depth);
    bool removeInternal(SCPINode *node, int depth);
    bool addInternal(SCPICommand *cmd, int depth);
    bool removeInternal(SCPICommand *cmd, int depth);
    QString name;
    std::vector<SCPINode*> subnodes;
    std::vector<SCPICommand*> commands;
    SCPINode *parent;
    bool operationPending;
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
        CmdError,
        QueryError,
        ExecError,
        False,
        True
    };

    static QString getResultName(SCPI::Result r);

    // call whenever a subnode completes an operation
    void someOperationCompleted();

public slots:
    void input(QString line);
    void process();
signals:
    void output(QString line);

private:

    enum class Flag  {
        OPC = 0x01, // Operation complete
        RQC = 0x02, // device wants to become the controller (of the bus)
        QYE = 0x04, // query error
        DDE = 0x08, // device-dependent error
        EXE = 0x10, // execution error
        CME = 0x20, // command error
        URQ = 0x40, // user request
        PON = 0x80, // power on
    };

    void setFlag(Flag flag);
    void clearFlag(Flag flag);
    bool getFlag(Flag flag);

    unsigned int SESR;
    unsigned int ESE;

    bool OCAS;
    bool OPCsetBitScheduled;
    bool OPCQueryScheduled;
    bool WAIexecuting;

    QList<QString> cmdQueue;
    bool processing;
    QSemaphore semQueue;
    QSemaphore semProcessing;
};

#endif // SCPI_H
