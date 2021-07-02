#include "scpi.h"
#include <QDebug>

SCPI::SCPI() :
    SCPINode("")
{
    lastNode = this;
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
     || alternateName(s1).compare(alternateName(s2), Qt::CaseInsensitive) == 0
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

bool SCPI::paramToULong(QStringList params, int index, unsigned long &dest)
{
    if(index >= params.size()) {
        return false;
    }
    bool okay;
    dest = params[index].toULong(&okay);
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

void SCPI::input(QString line)
{
    auto cmds = line.split(";");
    for(auto cmd : cmds) {
        if(cmd[0] == ':' || cmd[0] == '*') {
            // reset to root node
            lastNode = this;
        }
        if(cmd[0] == ':') {
            cmd.remove(0, 1);
        }
        cmd = cmd.toUpper();
        auto response = lastNode->parse(cmd, lastNode);
        emit output(response);
    }
}

bool SCPINode::add(SCPINode *node)
{
    if(nameCollision(node->name)) {
        qWarning() << "Unable to add SCPI node, name collision: " << node->name;
        return false;
    }
    subnodes.push_back(node);
    return true;
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
            if(SCPI::match(n->name, subnode)) {
                // pass on to next level
                return n->parse(cmd.right(cmd.size() - splitPos - 1), lastNode);
            }
        }
        // unable to find subnode
        return "ERROR";
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
            if(SCPI::match(c->name(), cmd)) {
                // save current node in case of non-root for the next command
                lastNode = this;
                if(isQuery) {
                    return c->query(params);
                } else {
                    return c->execute(params);
                }
            }
        }
        // couldn't find command
        return "ERROR";
    }
}

QString SCPICommand::execute(QStringList params)
{
    if(fn_cmd == nullptr) {
        return "ERROR";
    } else {
        return fn_cmd(params);
    }
}

QString SCPICommand::query(QStringList params)
{
    if(fn_query == nullptr) {
        return "ERROR";
    } else {
        return fn_query(params);
    }
}
