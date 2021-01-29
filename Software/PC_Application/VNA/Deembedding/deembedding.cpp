#include "deembedding.h"
#include "deembeddingdialog.h"
#include <QDebug>

using namespace std;

void Deembedding::configure()
{
    auto d = new DeembeddingDialog(this);
    d->show();
}

void Deembedding::Deembed(Protocol::Datapoint &d)
{
    for(auto it = options.begin();it != options.end();it++) {
        (*it)->transformDatapoint(d);
    }
}

void Deembedding::removeOption(unsigned int index)
{
    if(index < options.size()) {
        delete options[index];
        options.erase(options.begin() + index);
    }
}

void Deembedding::addOption(DeembeddingOption *option)
{
    options.push_back(option);
    connect(option, &DeembeddingOption::deleted, [=](DeembeddingOption *o){
        // find deleted option and remove from list
        auto pos = find(options.begin(), options.end(), o);
        if(pos != options.end()) {
            options.erase(pos);
        }
    });
}

void Deembedding::swapOptions(unsigned int index)
{
    if(index + 1 >= options.size()) {
        return;
    }
    std::swap(options[index], options[index+1]);
}

nlohmann::json Deembedding::toJSON()
{
    nlohmann::json list;
    for(auto m : options) {
        nlohmann::json jm;
        jm["operation"] = DeembeddingOption::getName(m->getType()).toStdString();
        jm["settings"] = m->toJSON();
        list.push_back(jm);
    }
    return list;
}

void Deembedding::fromJSON(nlohmann::json j)
{
    // clear all options
    while(options.size() > 0) {
        removeOption(0);
    }
    for(auto jm : j) {
        QString operation = QString::fromStdString(jm.value("operation", ""));
        if(operation.isEmpty()) {
            qWarning() << "Skipping empty de-embedding operation";
            continue;
        }
        // attempt to find the type of operation
        DeembeddingOption::Type type = DeembeddingOption::Type::Last;
        for(unsigned int i=0;i<(int) DeembeddingOption::Type::Last;i++) {
            if(DeembeddingOption::getName((DeembeddingOption::Type) i) == operation) {
                // found the correct operation
                type = (DeembeddingOption::Type) i;
                break;
            }
        }
        if(type == DeembeddingOption::Type::Last) {
            // unable to find this operation
            qWarning() << "Unable to create de-embedding operation:" << operation;
            continue;
        }
        qDebug() << "Creating math operation of type:" << operation;
        auto op = DeembeddingOption::create(type);
        if(jm.contains("settings")) {
            op->fromJSON(jm["settings"]);
        }
        addOption(op);
    }
}
