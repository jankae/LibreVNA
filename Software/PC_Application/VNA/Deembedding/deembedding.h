#ifndef DEEMBEDDING_H
#define DEEMBEDDING_H

#include "deembeddingoption.h"
#include <vector>
#include <QObject>
#include "savable.h"

class Deembedding : public QObject, public Savable
{
    Q_OBJECT
public:
    Deembedding(){};
    ~Deembedding(){};

    void Deembed(Protocol::Datapoint &d);

    void removeOption(unsigned int index);
    void addOption(DeembeddingOption* option);
    void swapOptions(unsigned int index);
    std::vector<DeembeddingOption*>& getOptions() {return options;};
    nlohmann::json toJSON() override;
    void fromJSON(nlohmann::json j) override;
public slots:
    void configure();

private:
    std::vector<DeembeddingOption*> options;
};

#endif // DEEMBEDDING_H
