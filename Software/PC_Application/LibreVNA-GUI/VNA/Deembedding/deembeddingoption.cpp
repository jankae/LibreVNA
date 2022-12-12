#include "deembeddingoption.h"

#include "portextension.h"
#include "twothru.h"
#include "matchingnetwork.h"
#include "impedancerenormalization.h"

DeembeddingOption *DeembeddingOption::create(DeembeddingOption::Type type)
{
    switch(type) {
    case Type::PortExtension:
        return new PortExtension();
    case Type::TwoThru:
        return new TwoThru();
    case Type::MatchingNetwork:
        return new MatchingNetwork();
    case Type::ImpedanceRenormalization:
        return new ImpedanceRenormalization();
    default:
        return nullptr;
    }
}

QString DeembeddingOption::TypeToString(DeembeddingOption::Type type)
{
    switch(type) {
    case Type::PortExtension:
        return "Port Extension";
    case Type::TwoThru:
        return "2xThru";
    case Type::MatchingNetwork:
        return "Matching Network";
    case Type::ImpedanceRenormalization:
        return "Impedance Renormalization";
    default:
        return "";
    }
}

DeembeddingOption::Type DeembeddingOption::TypeFromString(QString string)
{
    for(unsigned int i=0;i<(int) Type::Last;i++) {
        if(TypeToString((Type) i).compare(string, Qt::CaseInsensitive) == 0) {
            return (Type) i;
        }
    }
    return Type::Last;
}
