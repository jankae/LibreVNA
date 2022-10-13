#ifndef SAVABLE_H
#define SAVABLE_H

#include "json.hpp"
#include "Util/qpointervariant.h"

#include <QVariant>
#include <QDebug>
#include <QColor>

#include <array>
#include <exception>

class Savable {
public:
    virtual ~Savable(){}

    virtual nlohmann::json toJSON() = 0;
    virtual void fromJSON(nlohmann::json j) = 0;

    bool openFromFileDialog(QString title, QString filetype);
    bool saveToFileDialog(QString title, QString filetype, QString ending = "");

    using SettingDescription = struct {
        QPointerVariant var;
        QString name;
        QVariant def;
    };
    static void parseJSON(nlohmann::json j, std::vector<SettingDescription> descr) {
        for(auto e : descr) {
            auto list = e.name.split(".");
            auto *json_entry = &j;
            bool entry_exists = true;
            while(list.size() > 0) {
                auto key = list.takeFirst().toStdString();
                if((*json_entry).contains(key)) {
                    json_entry = &(*json_entry)[key];
                } else {
                    entry_exists = false;
                    break;
                }
            }
            if(!entry_exists) {
                // missing entry in json file, nothing to do (default values already set in constructor)
                qWarning() << "Entry" << e.name << "not present in file";
                e.var.setValue(e.def);
                continue;
            }
            // json library does not now about QVariant, handle used cases
            auto val = e.var.value();
            switch(static_cast<QMetaType::Type>(val.type())) {
            case QMetaType::Double: e.var.setValue((*json_entry).get<double>()); break;
            case QMetaType::Int: e.var.setValue((*json_entry).get<int>()); break;
            case QMetaType::Bool: e.var.setValue((*json_entry).get<bool>()); break;
            case QMetaType::QString: {
                auto s = QString::fromStdString((*json_entry).get<std::string>());
                e.var.setValue(s);
            }
                break;
            case QMetaType::QColor: {
                auto s = QString::fromStdString((*json_entry).get<std::string>());
                e.var.setValue(QColor(s));
            }
                break;
            default:
                throw std::runtime_error("Unimplemented metatype:"+std::string(val.typeName()));
            }
        }
    }
    static nlohmann::json createJSON(std::vector<SettingDescription> descr) {
        nlohmann::json j;
        for(auto e : descr) {
            auto list = e.name.split(".");
            auto *json_entry = &j;
            while(list.size() > 0) {
                json_entry = &(*json_entry)[list.takeFirst().toStdString()];
            }
            // json library does not now about QVariant, handle used cases
            auto val = e.var.value();
            switch(static_cast<QMetaType::Type>(val.type())) {
            case QMetaType::Double: *json_entry = val.toDouble(); break;
            case QMetaType::Int: *json_entry = val.toInt(); break;
            case QMetaType::Bool: *json_entry = val.toBool(); break;
            case QMetaType::QString: *json_entry = val.toString().toStdString(); break;
            case QMetaType::QColor: *json_entry = val.value<QColor>().name().toStdString(); break;
            default:
                throw std::runtime_error("Unimplemented metatype:"+std::string(val.typeName()));
            }
        }
        return j;
    }
};

#endif // SAVABLE_H
