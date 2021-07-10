#include "markergroup.h"

MarkerGroup::~MarkerGroup()
{
    while(markers.size()) {
        remove(*markers.begin());
    }
}

bool MarkerGroup::add(Marker *m)
{
    if(!applicable(m)) {
        // can't add to group
        return false;
    }

    if(m->getGroup()) {
        // marker already in other group remove from it
        m->getGroup()->remove(m);
    }

    connect(m, &Marker::positionChanged, this, &MarkerGroup::markerMoved);
    connect(m, &Marker::typeChanged, this, &MarkerGroup::checkMarker);
    connect(m, &Marker::domainChanged, this, &MarkerGroup::checkMarker);
    connect(m, &Marker::deleted, [=](){
        remove(m);
    });

    if(markers.size() > 0) {
        m->setPosition((*markers.begin())->getPosition());
    }

    m->setGroup(this);
    markers.insert(m);

    return true;
}

bool MarkerGroup::remove(Marker *m)
{
    if(markers.count(m)) {
        // todo break connections, notify marker
        disconnect(m, &Marker::positionChanged, this, &MarkerGroup::markerMoved);
        disconnect(m, &Marker::typeChanged, this, &MarkerGroup::checkMarker);
        disconnect(m, &Marker::domainChanged, this, &MarkerGroup::checkMarker);

        m->setGroup(nullptr);

        markers.erase(m);

        if(markers.size() == 0) {
           // all markers removed
            emit emptied(this);
        }

        return true;
    } else {
        // not in group
        return false;
    }
}

bool MarkerGroup::applicable(Marker *m)
{
    if(!m->isMovable()) {
        // can't add automatic markers to group
        return false;
    }
    if(markers.size() == 0) {
        // first marker in group
        domain = m->getDomain();
    } else {
        // check domain
        if(domain != m->getDomain()) {
            // only markers of the same domain are allowed in a group
            return false;
        }
    }
    return true;
}

void MarkerGroup::markerMoved(double newpos)
{
    // moving the other markers will trigger additional calls to this slot.
    // Set and check guard variable
    if(adjustingMarkers) {
        return;
    }
    adjustingMarkers = true;
    for(auto m : markers) {
        m->setPosition(newpos);
    }
    adjustingMarkers = false;
}

void MarkerGroup::checkMarker(Marker *m)
{
    if(!applicable(m)) {
        remove(m);
    }
}

unsigned int MarkerGroup::getNumber() const
{
    return number;
}
