#ifndef ESERIES_H
#define ESERIES_H


class ESeries
{
public:
    enum class Series {
        Ideal = 0,
        E6 = 1,
        E12 = 2,
        E24 = 3,
        E48 = 4,
        E96 = 5,
    };
    enum class Type {
        BestMatch = 0,
        Lower = 1,
        Higher = 2,
    };

    static double ToESeries(double value, Series s, Type t = Type::BestMatch);
};

#endif // ESERIES_H
