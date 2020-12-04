#ifndef SAVABLE_H
#define SAVABLE_H

#include "json.hpp"

class Savable {
public:
    virtual nlohmann::json toJSON() = 0;
    virtual void fromJSON(nlohmann::json j) = 0;
};

#endif // SAVABLE_H
