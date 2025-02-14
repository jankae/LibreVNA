#include "parameters.h"

using namespace std;

Sparam::Sparam(const Tparam &t) {
    data = Eigen::MatrixXcd(2,2);
    set(1,1, t.get(1,2) / t.get(2,2));
    set(2,1, Type(1) / t.get(2,2));
    set(1,2, (t.get(1,1)*t.get(2,2) - t.get(1,2)*t.get(2,1)) / t.get(2,2));
    set(2,2, -t.get(2,1) / t.get(2,2));
}

Sparam::Sparam(const ABCDparam &a, Type Z01, Type Z02) {
    data = Eigen::MatrixXcd(2,2);
    auto denom = a.get(1,1)*Z02+a.get(1,2)+a.get(2,1)*Z01*Z02+a.get(2,2)*Z01;
    set(1,1, (a.get(1,1)*Z02+a.get(1,2)-a.get(2,1)*conj(Z01)*Z02-a.get(2,2)*conj(Z01)) / denom);
    set(1,2, (2.0*(a.get(1,1)*a.get(2,2)-a.get(1,2)*a.get(2,1))*sqrt(real(Z01)*real(Z02))) / denom);
    set(2,1, (2.0*sqrt(real(Z01)*real(Z02))) / denom);
    set(2,2, (-a.get(1,1)*conj(Z02)+a.get(1,2)-a.get(2,1)*Z01*conj(Z02)+a.get(2,2)*Z01) / denom);
}

Sparam::Sparam(const ABCDparam &a, Type Z0)
    : Sparam(a, Z0, Z0)
{
}

void Sparam::swapPorts(unsigned int p1, unsigned int p2)
{
    // swap columns
    data.col(p1-1).swap(data.col(p2-1));
    data.row(p1-1).swap(data.row(p2-1));
    // auto cbuf = data.col(p1-1);
    // data.col(p1-1) = data.col(p2-1);
    // data.col(p2-1) = cbuf;
    // // swap rows
    // auto rbuf = data.row(p1-1);
    // data.row(p1-1) = data.row(p2-1);
    // data.row(p2-1) = rbuf;
}

ABCDparam::ABCDparam(const Sparam &s, Type Z01, Type Z02)
{
    if(s.ports() != 2) {
        throw std::runtime_error("Can only create ABCD parameter from 2 port S parameters");
    }
    data = Eigen::MatrixXcd(2,2);
    auto denom = 2.0*s.get(2,1)*sqrt(real(Z01)*real(Z02));
    set(1,1, ((conj(Z01)+s.get(1,1)*Z01)*(1.0-s.get(2,2))+s.get(1,2)*s.get(2,1)*Z01) / denom);
    set(1,2, ((conj(Z01)+s.get(1,1)*Z01)*(conj(Z02)+s.get(2,2)*Z02)-s.get(1,2)*s.get(2,1)*Z01*Z02) / denom);
    set(2,1, ((1.0-s.get(1,1))*(1.0-s.get(2,2))-s.get(1,2)*s.get(2,1)) / denom);
    set(2,2, ((1.0-s.get(1,1))*(conj(Z02)+s.get(2,2)*Z02)+s.get(1,2)*s.get(2,1)*Z02) / denom);
}

Tparam::Tparam(const Sparam &s)
{
    if(s.ports() != 2) {
        throw std::runtime_error("Can only create ABCD parameter from 2 port S parameters");
    }
    data = Eigen::MatrixXcd(2,2);
    set(1,1, -(s.get(1,1)*s.get(2,2) - s.get(1,2)*s.get(2,1)) / s.get(2,1));
    set(1,2, s.get(1,1) / s.get(2,1));
    set(2,1, -s.get(2,2) / s.get(2,1));
    set(2,2, 1.0 / s.get(2,1));
}

ABCDparam::ABCDparam(const Sparam &s, Type Z0)
    : ABCDparam(s, Z0, Z0)
{
}

Parameters::Parameters(Type m11, Type m12, Type m21, Type m22)
    : Parameters(2)
{
    data(0, 0) = m11;
    data(0, 1) = m12;
    data(1, 0) = m21;
    data(1, 1) = m22;
}

Parameters::Parameters(int num_ports)
{
    data = Eigen::MatrixXd::Zero(num_ports, num_ports);
}

nlohmann::json Parameters::toJSON()
{
    nlohmann::json ret;
    for(unsigned int i=0;i<data.cols();i++) {
        for(unsigned int j=0;j<data.cols();j++) {
            QString s_real = "m"+QString::number(i+1)+QString::number(j+1)+"_real";
            QString s_imag = "m"+QString::number(i+1)+QString::number(j+1)+"_imag";
            ret[s_real.toStdString()] = data(i, j).real();
            ret[s_imag.toStdString()] = data(i, j).imag();
        }
    }
    return ret;
}

void Parameters::fromJSON(nlohmann::json j)
{
    // figure out how many ports we need
    unsigned int max_port = 0;
    for(auto names : j.items()) {
        auto key = QString::fromStdString(names.key());
        unsigned int i = key.mid(1, 1).toInt();
        unsigned int j = key.mid(2, 1).toInt();
        if(i > max_port) {
            max_port = i;
        }
        if(j > max_port) {
            max_port = j;
        }
    }
    data = Eigen::MatrixXcd(max_port, max_port);
    for(unsigned int i=0;i<data.cols();i++) {
        for(unsigned int _j=0;_j<data.cols();_j++) {
            std::string s_real = ("m"+QString::number(i+1)+QString::number(_j+1)+"_real").toStdString();
            std::string s_imag = ("m"+QString::number(i+1)+QString::number(_j+1)+"_imag").toStdString();
            if(j.contains(s_real) && j.contains((s_imag))) {
                data(i, _j) = complex<double>(j.value(s_real, 0.0), j.value(s_imag, 0.0));
            } else {
                // no data, set to zero
                data(i, _j) = 0;
            }
        }
    }
}

Yparam::Yparam(const Sparam &s, Type Z01, Type Z02)
{
    // TODO can this be done for any number of ports
    if(s.ports() != 2) {
        throw std::runtime_error("Can only create ABCD parameter from 2 port S parameters");
    }
    data = Eigen::MatrixXcd(2,2);
    // from https://www.rfcafe.com/references/electrical/s-h-y-z.htm
    auto denom = (conj(Z01)+s.get(1,1)*Z01)*(conj(Z02)+s.get(2,2)*Z02)-s.get(1,2)*s.get(2,1)*Z01*Z02;
    set(1,1, ((1.0-s.get(1,1))*(conj(Z02)+s.get(2,2)*Z02)+s.get(1,2)*s.get(2,1)*Z02) / denom);
    set(1,2, -2.0*s.get(1,2)*sqrt(real(Z01)*real(Z02)));
    set(2,1, -2.0*s.get(2,1)*sqrt(real(Z01)*real(Z02)));
    set(2,2, ((conj(Z01)+s.get(1,1)*Z01)*(1.0-s.get(2,2))+s.get(1,2)*s.get(2,1)*Z01) / denom);
}

Yparam::Yparam(const Sparam &s, Type Z0)
    : Yparam(s, Z0, Z0)
{
}
