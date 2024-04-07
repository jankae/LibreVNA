#include "scpi.h"

#include <QDebug>

SCPI::SCPI() :
    SCPINode("")
{
    WAIexecuting = false;
    OPCQueryScheduled = false;
    OCAS = false;
    ESR = 0x00;

    add(new SCPICommand("*OPC", [=](QStringList){
        // OPC command
        if(isOperationPending()) {
            OCAS = true;
        }
        return SCPI::getResultName(SCPI::Result::Empty);
    }, [=](QStringList) -> QString {
        // OPC query
        if(isOperationPending()) {
            // operation pending
            OPCQueryScheduled = true;
            OCAS = true;
            return SCPI::getResultName(SCPI::Result::Empty);
        } else {
            // no operation, can return immediately
            OCAS = false;
            setFlag(Flag::OPC);
            return "1";
        }
    }));

    add(new SCPICommand("*WAI", [=](QStringList){
        // WAI command
        if(isOperationPending()) {
            WAIexecuting = true;
        }
        return SCPI::getResultName(SCPI::Result::Empty);
    }, nullptr));

    add(new SCPICommand("*LST", nullptr, [=](QStringList){
        QString list;
        createCommandList("", list);
        return list.trimmed();
    }));
}

bool SCPI::match(QString s1, QString s2)
{
    if (s1.compare(s2, Qt::CaseInsensitive) == 0
     || s1.compare(alternateName(s2), Qt::CaseInsensitive) == 0
     || alternateName(s1).compare(s2, Qt::CaseInsensitive) == 0
     || alternateName(s1).compare(alternateName(s2), Qt::CaseInsensitive) == 0) {
        return true;
    } else {
        return false;
    }
}

QString SCPI::alternateName(QString name)
{
    while(name.size() > 0 && name[name.size()-1].isLower()) {
        name.chop(1);
    }
    return name;
}

bool SCPI::paramToDouble(QStringList params, int index, double &dest)
{
    if(index >= params.size()) {
        return false;
    }
    bool okay;
    dest = params[index].toDouble(&okay);
    return okay;
}

bool SCPI::paramToULongLong(QStringList params, int index, unsigned long long &dest)
{
    if(index >= params.size()) {
        return false;
    }
    bool okay;
    dest = params[index].toULongLong(&okay);
    return okay;
}

bool SCPI::paramToLong(QStringList params, int index, long &dest)
{
    if(index >= params.size()) {
        return false;
    }
    bool okay;
    dest = params[index].toLong(&okay);
    return okay;
}

bool SCPI::paramToBool(QStringList params, int index, bool &dest)
{
    if(index >= params.size()) {
        return false;
    }
    bool okay = false;
    if(params[index] == "TRUE") {
        dest = true;
        okay = true;
    } else if(params[index] == "FALSE") {
        dest = false;
        okay = true;
    }
    return okay;
}

QString SCPI::getResultName(SCPI::Result r)
{
    switch (r) {
    case Result::Empty:
        return "";
    case Result::Error:
    default:
        return "ERROR";
    case Result::False:
        return "FALSE";
    case Result::True:
        return "TRUE";
    }
}

void SCPI::input(QString line)
{
    cmdQueue.append(line);
    process();
}

void SCPI::process()
{
    while(!WAIexecuting && !cmdQueue.isEmpty()) {
        auto cmd = cmdQueue.front();
        cmdQueue.pop_front();
        auto cmds = cmd.split(";");
        SCPINode *lastNode = this;
        for(auto cmd : cmds) {
            if(cmd.size() > 0) {
                if(cmd[0] == ':' || cmd[0] == '*') {
                    // reset to root node
                    lastNode = this;
                }
                if(cmd[0] == ':') {
                    cmd.remove(0, 1);
                }
                auto response = lastNode->parse(cmd, lastNode);
                emit output(response);
            }
        }
    }
}

void SCPI::someOperationCompleted()
{
    if(!isOperationPending()) {
        // all operations are complete
        if(OCAS) {
            OCAS = false;
            setFlag(Flag::OPC);
            if(OPCQueryScheduled) {
                output("1");
                OPCQueryScheduled = false;
            }
        }
        if(WAIexecuting) {
            WAIexecuting = false;
            // process any queued commands
            process();
        }
    }
}

void SCPI::setFlag(Flag flag)
{
    ESR |= ((int) flag);
}

void SCPI::clearFlag(Flag flag)
{
    ESR &= ~((int) flag);
}

bool SCPI::getFlag(Flag flag)
{
    return ESR & (int) flag;
}

SCPINode::~SCPINode()
{
    if(parent) {
        parent->remove(this);
    }
    while(commands.size() > 0) {
        delete commands.front();
        commands.erase(commands.begin());
    }
    while(subnodes.size() > 0) {
        auto node = subnodes.front();
        remove(node);
        delete node;
    }
}

bool SCPINode::add(SCPINode *node)
{
    if(nameCollision(node->name)) {
        qWarning() << "Unable to add SCPI node, name collision: " << node->name;
        return false;
    }
    subnodes.push_back(node);
    node->parent = this;
    return true;
}

bool SCPINode::remove(SCPINode *node)
{
    auto it = std::find(subnodes.begin(), subnodes.end(), node);
    if(it != subnodes.end()) {
        subnodes.erase(it);
        node->parent = nullptr;
        return true;
    } else {
        return false;
    }
}

bool SCPINode::add(SCPICommand *cmd)
{
    if(nameCollision(cmd->name())) {
        qWarning() << "Unable to add SCPI node, name collision: " << cmd->name();
        return false;
    }
    commands.push_back(cmd);
    return true;
}

bool SCPINode::addDoubleParameter(QString name, double &param, bool gettable, bool settable, std::function<void(void)> setCallback)
{
    auto cmd = settable ? [&param, setCallback](QStringList params) -> QString {
        if(SCPI::paramToDouble(params, 0, param)) {
            if(setCallback) {
                setCallback();
            }
            return SCPI::getResultName(SCPI::Result::Empty);
        } else {
            return SCPI::getResultName(SCPI::Result::Error);
        }
    } : (std::function<QString(QStringList)>) nullptr;
    auto query = gettable ? [=](QStringList params) -> QString {
        Q_UNUSED(params)
        return QString::number(param);
    } : (std::function<QString(QStringList)>) nullptr;
    return add(new SCPICommand(name, cmd, query));
}

bool SCPINode::addUnsignedIntParameter(QString name, unsigned int &param, bool gettable, bool settable, std::function<void(void)> setCallback)
{
    auto cmd = settable ? [&param, setCallback](QStringList params) -> QString {
        unsigned long long value;
        if(SCPI::paramToULongLong(params, 0, value)) {
            param = value;
            if(setCallback) {
                setCallback();
            }
            return SCPI::getResultName(SCPI::Result::Empty);
        } else {
            return SCPI::getResultName(SCPI::Result::Error);
        }
    } : (std::function<QString(QStringList)>) nullptr;
    auto query = gettable ? [=](QStringList params) -> QString {
        Q_UNUSED(params)
        return QString::number(param);
    } : (std::function<QString(QStringList)>) nullptr;
    return add(new SCPICommand(name, cmd, query));
}

bool SCPINode::addBoolParameter(QString name, bool &param, bool gettable, bool settable, std::function<void(void)> setCallback)
{
    auto cmd = settable ? [&param, setCallback](QStringList params) -> QString {
        if(SCPI::paramToBool(params, 0, param)) {
            if(setCallback) {
                setCallback();
            }
            return SCPI::getResultName(SCPI::Result::Empty);
        } else {
            return SCPI::getResultName(SCPI::Result::Error);
        }
    } : (std::function<QString(QStringList)>) nullptr;
    auto query = gettable ? [=](QStringList params) -> QString {
        Q_UNUSED(params)
        return param ? SCPI::getResultName(SCPI::Result::True) : SCPI::getResultName(SCPI::Result::False);
    } : (std::function<QString(QStringList)>) nullptr;
    return add(new SCPICommand(name, cmd, query));
}

bool SCPINode::changeName(QString newname)
{
    if(newname == name) {
        return true;
    }
    if(parent) {
        if(parent->nameCollision(newname)) {
            // new name would result in a collision
            return false;
        }
    }
    name = newname;
    return true;
}

void SCPINode::setOperationPending(bool pending)
{
    if(operationPending != pending) {
        operationPending = pending;
        if(!operationPending) {
            // operation completed, needs to perform check if all operations are complete
            auto root = this;
            while(root->parent) {
                root = root->parent;
            }
            auto scpi = static_cast<SCPI*>(root);
            scpi->someOperationCompleted();
        }
    }
}

bool SCPINode::isOperationPending()
{
    if(operationPending) {
        return true;
    }
    for(auto node : subnodes) {
        if(node->isOperationPending()) {
            return true;
        }
    }
    // no node has any pending operations
    return false;
}

bool SCPINode::nameCollision(QString name)
{
    for(auto n : subnodes) {
        if(SCPI::match(n->name, name)) {
            return  true;
        }
    }
    for(auto c : commands) {
        if(SCPI::match(c->name(), name)) {
            return  true;
        }
    }
    return false;
}

void SCPINode::createCommandList(QString prefix, QString &list)
{
    for(auto c : commands) {
        if(c->queryable()) {
            list += prefix + c->name() + "?\n";
        }
        if(c->executable()) {
            list += prefix + c->name() + '\n';
        }
    }
    for(auto n : subnodes) {
        n->createCommandList(prefix + n->name + ":", list);
    }
}

QString SCPINode::parse(QString cmd, SCPINode* &lastNode)
{
    if(cmd.isEmpty()) {
        return "";
    }
    auto splitPos = cmd.indexOf(':');
    if(splitPos > 0) {
        // have not reached a leaf, find next subnode
        auto subnode = cmd.left(splitPos);
        for(auto n : subnodes) {
            if(SCPI::match(n->name, subnode.toUpper())) {
                // pass on to next level
                return n->parse(cmd.right(cmd.size() - splitPos - 1), lastNode);
            }
        }
        // unable to find subnode
        return SCPI::getResultName(SCPI::Result::Error);
    } else {
        // no more levels, search for command
        auto params = cmd.split(" ");
        auto cmd = params.front();
        params.pop_front();
        bool isQuery = false;
        if (cmd[cmd.size()-1]=='?') {
            isQuery = true;
            cmd.chop(1);
        }
        for(auto c : commands) {
            if(SCPI::match(c->name(), cmd.toUpper())) {
                // save current node in case of non-root for the next command
                lastNode = this;
                if(c->convertToUppercase()) {
                    for(auto &p : params) {
                        p = p.toUpper();
                    }
                }
                if(isQuery) {
                    return c->query(params);
                } else {
                    return c->execute(params);
                }
            }
        }
        // couldn't find command
        return SCPI::getResultName(SCPI::Result::Error);
    }
}

QString SCPICommand::execute(QStringList params)
{
    if(fn_cmd == nullptr) {
        return SCPI::getResultName(SCPI::Result::Error);
    } else {
        return fn_cmd(params);
    }
}

QString SCPICommand::query(QStringList params)
{
    if(fn_query == nullptr) {
        return SCPI::getResultName(SCPI::Result::Error);
    } else {
        return fn_query(params);
    }
}
