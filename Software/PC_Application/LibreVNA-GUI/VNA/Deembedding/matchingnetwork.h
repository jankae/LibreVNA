#ifndef MATCHINGNETWORK_H
#define MATCHINGNETWORK_H

#include "CustomWidgets/siunitedit.h"
#include "deembeddingoption.h"
#include "Tools/parameters.h"
#include "savable.h"
#include "touchstone.h"

#include <QWidget>
#include <QLabel>
#include <vector>


class MatchingComponent : public QFrame, public Savable, public SCPINode
{
    Q_OBJECT
public:
    enum class Type {
        SeriesR,
        SeriesL,
        SeriesC,
        ParallelR,
        ParallelL,
        ParallelC,
        DefinedThrough,
        DefinedShunt,
        // Add new matching components here, do not explicitly assign values and keep the Last entry at the last position
        Last,
    };

    MatchingComponent(Type type);
    ~MatchingComponent();
    ABCDparam parameters(double freq);
    void setValue(double v);

    static MatchingComponent* createFromName(QString name);
    QString getName();

    nlohmann::json toJSON() override;
    void fromJSON(nlohmann::json j) override;

signals:
    void valueChanged();
    void deleted(MatchingComponent* m);
protected:
    SIUnitEdit *eValue;
    Touchstone *touchstone;
    QLabel *touchstoneLabel;
private:
    void mouseDoubleClickEvent(QMouseEvent *e) override;
    void updateTouchstoneLabel();
    static QString typeToName(Type type);
    Type type;
    void keyPressEvent(QKeyEvent *event) override;
    void focusInEvent(QFocusEvent *event) override;
    void focusOutEvent(QFocusEvent *event) override;
    QString oldStylesheet;
    double value;
};

class MatchingNetwork : public DeembeddingOption
{
public:
    MatchingNetwork();
    ~MatchingNetwork();

    // DeembeddingOption interface
public:
    std::set<unsigned int> getAffectedPorts() override;
    void transformDatapoint(DeviceDriver::VNAMeasurement &p) override;
    void edit() override;
    Type getType() override {return Type::MatchingNetwork;}
    nlohmann::json toJSON() override;
    void fromJSON(nlohmann::json j) override;

    void clearNetwork();
private:
    static constexpr int imageHeight = 151;
    static constexpr int componentWidth = 151;
    static constexpr int DUTWidth = 151;
    static constexpr int portWidth = 76;
    MatchingComponent *componentAtPosition(int pos);
    unsigned int findInsertPosition(int xcoord);
    void addComponentAtPosition(int pos, MatchingComponent *c);
    void addComponent(int index, MatchingComponent *c);
    void removeComponent(int index);
    void removeComponent(MatchingComponent *c);
    void createDragComponent(MatchingComponent *c);
    void updateInsertIndicator(int xcoord);
    bool eventFilter(QObject *object, QEvent *event) override;

    void updateSCPINames();

    std::vector<MatchingComponent*> network;
    unsigned int port;

    QWidget *graph, *insertIndicator;
    QPoint dragStartPosition;
    MatchingComponent *dragComponent;
    bool dropPending;
    MatchingComponent *dropComponent;

    class MatchingPoint {
    public:
        ABCDparam forward;
        ABCDparam reverse;
    };
    std::map<double, MatchingPoint> matching;

    bool addNetwork;
};

#endif // MATCHINGNETWORK_H
