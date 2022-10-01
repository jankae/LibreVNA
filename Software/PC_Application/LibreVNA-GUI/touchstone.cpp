#include "touchstone.h"

#include "Util/util.h"

#include <limits>
#include <algorithm>
#include <fstream>
#include <iomanip>
#include <cmath>
#include <cctype>
#include <string>
#include <QDebug>

using namespace std;

Touchstone::Touchstone(unsigned int ports)
{
    this->m_ports = ports;
    referenceImpedance = 50.0;
    m_datapoints.clear();
}

void Touchstone::AddDatapoint(Touchstone::Datapoint p)
{
    if (p.S.size() != m_ports * m_ports) {
        throw runtime_error("Invalid number of parameters");
    }

    bool needs_sort = false;
    if (m_datapoints.size() > 0 && m_datapoints.back().frequency >= p.frequency) {
        needs_sort = true;
    }
    m_datapoints.push_back(p);
    if(needs_sort) {
        sort(m_datapoints.begin(), m_datapoints.end(), [](Datapoint &a, Datapoint &b) {
           return a.frequency < b.frequency;
        });
    }
}

void Touchstone::toFile(QString filename, Scale unit, Format format)
{
    // add correct file extension if not already present
    QString extension = ".s"+QString::number(m_ports)+"p";
    if(!filename.endsWith(extension)) {
        filename.append(extension);
    }

    // create file
    ofstream file;
    file.open(filename.toStdString());

    file << toString(unit, format).rdbuf();

    file.close();
    this->filename = filename;
}

stringstream Touchstone::toString(Touchstone::Scale unit, Touchstone::Format format)
{
    stringstream s;
    s << std::fixed << std::setprecision(12);
    // write option line
    s << "# ";
    switch(unit) {
        case Scale::Hz: s << "HZ "; break;
        case Scale::kHz: s << "KHZ "; break;
        case Scale::MHz: s << "MHZ "; break;
        case Scale::GHz: s << "GHZ "; break;
    }
    // only S parameters supported so far
    s << "S ";
    switch(format) {
        case Format::DBAngle: s << "DB "; break;
        case Format::RealImaginary: s << "RI "; break;
        case Format::MagnitudeAngle: s << "MA "; break;
    }
    // reference impedance
    s << "R " << referenceImpedance << "\n";

    auto printParameter = [format](ostream &out, complex<double> &c) {
        switch (format) {
        case Format::RealImaginary:
            out << c.real() << " " << c.imag();
            break;
        case Format::MagnitudeAngle:
            out << abs(c) << " " << arg(c) / M_PI * 180.0;
            break;
        case Format::DBAngle:
            out << Util::SparamTodB(c) << " " << arg(c) / M_PI * 180.0;
            break;
        }
    };

    for(auto p : m_datapoints) {
        switch(unit) {
            case Scale::Hz: s << p.frequency; break;
            case Scale::kHz: s << p.frequency / 1e3; break;
            case Scale::MHz: s << p.frequency / 1e6; break;
            case Scale::GHz: s << p.frequency / 1e9; break;
        }
        s << " ";
        // special cases for 1 and 2 port
        if (m_ports == 1) {
            printParameter(s, p.S[0]);
            s << "\n";
        } else if (m_ports == 2){
            printParameter(s, p.S[0]);
            // touchstone expects S11 S21 S12 S22 order, swap S12 and S21
            s << " ";
            printParameter(s, p.S[2]);
            s << " ";
            printParameter(s, p.S[1]);
            s << " ";
            printParameter(s, p.S[3]);
            s << "\n";
        } else {
            // print parameters in matrix form
            for(unsigned int i=0;i<m_ports;i++) {
                for(unsigned int j=0;j<m_ports;j++) {
                    printParameter(s, p.S[i*m_ports + j]);
                    if (j%4 == 3) {
                        s << "\n";
                    } else {
                        s << " ";
                    }
                }
                if(m_ports%4 != 0) {
                    s << "\n";
                }
            }
        }
    }
    return s;
}

Touchstone Touchstone::fromFile(string filename)
{
    ifstream file;
    file.open(filename);

    if(!file.is_open()) {
        throw runtime_error("Unable to open file:" + filename);
    }

    // extract number of ports from filename
    auto index_extension = filename.find_last_of('.');
    if((filename[index_extension + 1] != 's' && filename[index_extension + 1] != 'S')
            || filename[index_extension+2] < '1' || filename[index_extension+2] > '9'
            || (filename[index_extension+3] != 'p' && filename[index_extension+3] != 'P')) {
        throw runtime_error("Invalid filename extension");
    }
    unsigned int ports = filename[index_extension + 2] - '0';
    auto ret = Touchstone(ports);

    Scale unit = Scale::GHz;
    Format format = Format::RealImaginary;

    bool option_line_found = false;
    unsigned int parameter_cnt = 0;

    Datapoint point;

    string line;
    while(getline(file, line)) {
        // remove comments
        auto comment = line.find_first_of('!');
        if(comment != string::npos) {
            line.erase(comment);
        }
        // remove leading whitespace
        size_t first = line.find_first_not_of(" \t");
        if (string::npos == first) {
            // string does only contain whitespace, skip line
            continue;
        }
        line.erase(0, first);

        if (line[0] == '#') {
            // this is the option line
            if (option_line_found) {
                throw runtime_error("Additional option line present");
            }
            option_line_found = true;
            transform(line.begin(), line.end(), line.begin(), ::toupper);
            // check individual options
            istringstream iss(line);
            bool last_R = false;
            string s;
            // throw away the option line start character
            iss >> s;
            for(;iss>>s;) {
                if(last_R) {
                    last_R = false;
                    // read reference impedance
                    ret.referenceImpedance = stod(s, nullptr);
                    break;
                }
                if (!s.compare("HZ")) {
                    unit = Scale::Hz;
                } else if (!s.compare("KHZ")) {
                    unit = Scale::kHz;
                } else if (!s.compare("MHZ")) {
                    unit = Scale::MHz;
                } else if (!s.compare("GHZ")) {
                    unit = Scale::GHz;
                } else if (!s.compare("S")) {
                    // S parameter, nothing to do
                } else if (!s.compare("Y")) {
                   throw runtime_error("Y parameters not supported");
                } else if (!s.compare("Z")) {
                    throw runtime_error("Z parameters not supported");
                } else if (!s.compare("G")) {
                    throw runtime_error("G parameters not supported");
                } else if (!s.compare("H")) {
                    throw runtime_error("H parameters not supported");
                } else if(!s.compare("MA")) {
                    format = Format::MagnitudeAngle;
                } else if(!s.compare("DB")) {
                    format = Format::DBAngle;
                } else if(!s.compare("RI")) {
                    format = Format::RealImaginary;
                } else if(!s.compare("R")) {
                    // next option is the reference impedance
                    last_R = true;
                } else {
                    throw runtime_error("Unexpected option in option line");
                }
            }
        } else {
            // not the option line
            if(!option_line_found) {
                throw runtime_error("First dataline before option line");
            }
            auto parseDatapoint = [format](istream &in) -> complex<double> {
                double part1, part2;
                in >> part1;
                in >> part2;
                complex<double> ret;
                switch(format) {
                case Format::MagnitudeAngle:
                    ret = polar(part1, part2 / 180.0 * M_PI);
                    break;
                case Format::DBAngle:
                    ret = polar(pow(10, part1/20), part2 / 180.0 * M_PI);
                    break;
                case Format::RealImaginary:
                    ret = complex<double>(part1, part2);
                    break;
                }
                return ret;
            };
            istringstream iss(line);
            if (parameter_cnt == 0) {
                iss >> point.frequency;
                point.S.clear();
                switch(unit) {
                    case Scale::Hz: break;
                    case Scale::kHz: point.frequency *= 1e3; break;
                    case Scale::MHz: point.frequency *= 1e6; break;
                    case Scale::GHz: point.frequency *= 1e9; break;
                }
            }
            unsigned int parameters_per_line;
            if(ports == 1) {
                parameters_per_line = 1;
            } else if(ports == 3) {
                parameters_per_line = 3;
            } else {
                parameters_per_line = 4;
            }
            unsigned int parameters_per_point = ports * ports;
            for(unsigned int i=0;i<parameters_per_line;i++) {
                point.S.push_back(parseDatapoint(iss));
                parameter_cnt++;
                if(parameter_cnt >= parameters_per_point) {
                    parameter_cnt = 0;
                    if(ports == 2) {
                        // 2 port touchstone has S11 S21 S12 S22 order, swap S12 and S21
                        swap(point.S[1], point.S[2]);
                    }
                    ret.AddDatapoint(point);
                    break;
                }
            }
        }
    }
    ret.filename = QString::fromStdString(filename);
    return ret;
}

double Touchstone::minFreq()
{
    if (m_datapoints.size() > 0) {
        return m_datapoints.front().frequency;
    } else {
        return numeric_limits<double>::quiet_NaN();
    }
}

double Touchstone::maxFreq()
{
    if (m_datapoints.size() > 0) {
        return m_datapoints.back().frequency;
    } else {
        return numeric_limits<double>::quiet_NaN();
    }
}

Touchstone::Datapoint Touchstone::interpolate(double frequency)
{
    if(m_datapoints.size() == 0) {
        throw runtime_error("Trying to interpolate empty touchstone data");
    }
    // Check if requested frequency is outside of points and return first/last datapoint respectively
    if(frequency <= m_datapoints.front().frequency) {
        return m_datapoints.front();
    } else if(frequency >= m_datapoints.back().frequency) {
        return m_datapoints.back();
    }
    // frequency within points, interpolate
    auto lower = lower_bound(m_datapoints.begin(), m_datapoints.end(), frequency, [](const Datapoint &lhs, double rhs) -> bool {
        return lhs.frequency < rhs;
    });
    auto highPoint = *lower;
    auto lowPoint = *prev(lower);
    double alpha = (frequency - lowPoint.frequency) / (highPoint.frequency - lowPoint.frequency);
    Datapoint ret;
    ret.frequency = frequency;
    for(unsigned int i=0;i<lowPoint.S.size();i++) {
        ret.S.push_back(lowPoint.S[i] * (1.0-alpha) + highPoint.S[i] * alpha);
    }
    return ret;
}

void Touchstone::reduceTo2Port(unsigned int port1, unsigned int port2)
{
    if (port1 >= m_ports || port2 >= m_ports || port1 == port2) {
        throw runtime_error("Invalid port number");
    }
    unsigned int S11_index = port1 * m_ports + port1;
    unsigned int S22_index = port2 * m_ports + port2;
    unsigned int S12_index = port1 * m_ports + port2;
    unsigned int S21_index = port2 * m_ports + port1;
    if(m_ports == 2) {
        swap(S21_index, S12_index);
    }
    for(auto &p : m_datapoints) {
        auto S11 = p.S[S11_index];
        auto S12 = p.S[S12_index];
        auto S21 = p.S[S21_index];
        auto S22 = p.S[S22_index];
        p.S.clear();
        p.S.push_back(S11);
        p.S.push_back(S21);
        p.S.push_back(S12);
        p.S.push_back(S22);
    }
    m_ports = 2;
}

void Touchstone::reduceTo1Port(unsigned int port)
{
    if (port >= m_ports) {
        throw runtime_error("Invalid port number");
    }
    if(m_ports == 1) {
        // already at one port, nothing to do
        return;
    }
    unsigned int S11_index = port * m_ports + port;
    for(auto &p : m_datapoints) {
        auto S11 = p.S[S11_index];
        p.S.clear();
        p.S.push_back(S11);
    }
    m_ports = 1;
}

QString Touchstone::getFilename() const
{
    return filename;
}

nlohmann::json Touchstone::toJSON()
{
    nlohmann::json j;
    j["ports"] = m_ports;
    j["filename"] = filename.toStdString();
    if(m_datapoints.size() > 0) {
        nlohmann::json json_points;
        for(auto d : m_datapoints) {
            nlohmann::json point;
            point["frequency"] = d.frequency;
            nlohmann::json sparams;
            for(auto s : d.S) {
                nlohmann::json sparam;
                sparam["real"] = s.real();
                sparam["imag"] = s.imag();
                sparams.push_back(sparam);
            }
            point["Sparams"] = sparams;
            json_points.push_back(point);
        }
        j["datapoints"] = json_points;
    }
    return j;
}

void Touchstone::fromJSON(nlohmann::json j)
{
    m_datapoints.clear();
    filename = QString::fromStdString(j.value("filename", ""));
    m_ports = j.value("ports", 0);
    if(!m_ports || !j.contains("datapoints")) {
        return;
    }
    auto json_points = j["datapoints"];
    for(auto point : json_points) {
        Datapoint d;
        if(!point.contains("frequency") || !point.contains("Sparams")) {
            // missing data, abort here
            qWarning() << "Touchstone data point does not contain frequency or S parameters";
            break;
        }
        d.frequency = point["frequency"];
        if(point["Sparams"].size() != m_ports * m_ports) {
            // invalid number of Sparams, abort here
            qWarning() << "Invalid number of S parameters, got" << point["Sparams"].size() << "expected" << m_ports*m_ports;
            break;
        }
        for(auto Sparam : point["Sparams"]) {
            d.S.push_back(complex<double>(Sparam.value("real", 0.0), Sparam.value("imag", 0.0)));
        }
        m_datapoints.push_back(d);
    }
}

double Touchstone::getReferenceImpedance() const
{
    return referenceImpedance;
}

void Touchstone::setReferenceImpedance(double value)
{
    referenceImpedance = value;
}

void Touchstone::setFilename(const QString &value)
{
    filename = value;
}
