#include "deembeddingoption.h"

#include "portextension.h"
#include "twothru.h"
#include "matchingnetwork.h"

DeembeddingOption *DeembeddingOption::create(DeembeddingOption::Type type)
{
    switch(type) {
    case Type::PortExtension:
        return new PortExtension();
    case Type::TwoThru:
        return new TwoThru();
    case Type::MatchingNetwork:
        return new MatchingNetwork();
    default:
        return nullptr;
    }
}

QString DeembeddingOption::getName(DeembeddingOption::Type type)
{
    switch(type) {
    case Type::PortExtension:
        return "Port Extension";
    case Type::TwoThru:
        return "2xThru";
    case Type::MatchingNetwork:
        return "Matching Network";
    default:
        return "";
    }
}
