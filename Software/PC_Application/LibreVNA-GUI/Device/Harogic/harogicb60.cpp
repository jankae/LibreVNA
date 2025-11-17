#include "harogicb60.h"

HarogicB60::HarogicB60()
{
    validUSBIDs.clear();
    validUSBIDs.append({0x367F, 0x0200, "B60"});

    for(auto &s : specificSettings) {
        s.name.replace("LibreVNAUSBDriver", "HarogicB60Driver");
    }
}

QString HarogicB60::getDriverName()
{
    return "Harogic B60";
}
