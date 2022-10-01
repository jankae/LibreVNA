#ifndef MARKERGROUP_H
#define MARKERGROUP_H

#include "marker.h"

#include <QObject>

class MarkerGroup : public QObject
{
    Q_OBJECT
public:
    MarkerGroup(unsigned int number)
        : adjustingMarkers(false),
          domain(Trace::DataType::Invalid),
          markers(),
          number(number){};
    ~MarkerGroup();

    bool add(Marker *m);

    unsigned int getNumber() const;
    bool applicable(Marker *m);

signals:
    void emptied(MarkerGroup*);

public slots:
    bool remove(Marker *m);

private:

    void markerMoved(double newpos);
    // Check if marker still applicable for group, remove if necessary
    void checkMarker(Marker *m);
    bool adjustingMarkers;
    Trace::DataType domain;
    std::set<Marker*> markers;
    unsigned int number;
};

#endif // MARKERGROUP_H
