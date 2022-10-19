#ifndef PRBS_H
#define PRBS_H


class PRBS
{
public:
    PRBS(unsigned int bits);

    bool next();

private:
    unsigned int bits;
    unsigned int shiftReg;
    unsigned int polynom;
};

#endif // PRBS_H
