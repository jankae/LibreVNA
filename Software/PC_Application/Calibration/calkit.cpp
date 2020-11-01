#include "calkit.h"

#include <fstream>
#include <iomanip>
#include "calkitdialog.h"
#include <math.h>

using namespace std;

Calkit::Calkit()
 : ts_open(nullptr),
   ts_short(nullptr),
   ts_load(nullptr),
   ts_through(nullptr),
   ts_cached(false)
{
    open_Z0 = 50.0;
    open_delay = 0.0;
    open_loss = 0.0;
    open_C0 = 0.0;
    open_C1 = 0.0;
    open_C2 = 0.0;
    open_C3 = 0.0;

    short_Z0 = 50.0;
    short_delay = 0.0;
    short_loss = 0.0;
    short_L0 = 0.0;
    short_L1 = 0.0;
    short_L2 = 0.0;
    short_L3 = 0.0;

    load_Z0 = 50.0;

    through_Z0 = 50.0;
    through_delay = 0.0;
    through_loss = 0.0;

    open_measurements = false;
    short_measurements = false;
    load_measurements = false;
    through_measurements = false;

    open_file = "";
    short_file = "";
    load_file = "";
    through_file = "";

    open_Sparam = 0;
    short_Sparam = 0;
    load_Sparam = 0;
    through_Sparam1 = 0;
    through_Sparam2 = 1;

    TRL_through_Z0 = 50.0;
    TRL_reflection_short = true;
    TRL_line_delay = 74;
    TRL_line_maxfreq = 6000000000;
    TRL_line_minfreq = 751000000;
}

void Calkit::toFile(std::string filename)
{
    ofstream file;
    file.open(filename);
    file << std::fixed << std::setprecision(12);
    file << open_measurements << "\n" << short_measurements << "\n" << load_measurements << "\n" << through_measurements << "\n";
    file << open_Z0 << "\n" << open_delay << "\n" << open_loss << "\n" << open_C0 << "\n" << open_C1 << "\n" << open_C2 << "\n" << open_C3 << "\n";
    file << short_Z0 << "\n" << short_delay << "\n" << short_loss << "\n" << short_L0 << "\n" << short_L1 << "\n" << short_L2 << "\n" << short_L3 << "\n";
    file << load_Z0 << "\n";
    file << through_Z0 << "\n" << through_delay << "\n" << through_loss << "\n";
    if(open_measurements) {
        file << open_file << "\n" << open_Sparam << "\n";
    }
    if(short_measurements) {
        file << short_file << "\n" << short_Sparam << "\n";
    }
    if(load_measurements) {
        file << load_file << "\n" << load_Sparam << "\n";
    }
    if(through_measurements) {
        file << through_file << "\n" << through_Sparam1 << "\n" << through_Sparam2 << "\n";
    }
    file << TRL_through_Z0 << "\n";
    file << TRL_reflection_short << "\n";
    file << TRL_line_delay << "\n";
    file << TRL_line_minfreq << "\n";
    file << TRL_line_maxfreq << "\n";
    file.close();
}

static QString readLine(ifstream &file) {
    string line;
    getline(file, line);
    return QString::fromStdString(line).simplified();
}

Calkit Calkit::fromFile(std::string filename)
{
    Calkit c;
    ifstream file;
    file.open(filename);
    if(!file.is_open()) {
        throw runtime_error("Unable to open file");
    }
    c.open_measurements = readLine(file).toInt();
    c.short_measurements = readLine(file).toInt();
    c.load_measurements = readLine(file).toInt();
    c.through_measurements = readLine(file).toInt();
    c.open_Z0 = readLine(file).toDouble();
    c.open_delay = readLine(file).toDouble();
    c.open_loss = readLine(file).toDouble();
    c.open_C0 = readLine(file).toDouble();
    c.open_C1 = readLine(file).toDouble();
    c.open_C2 = readLine(file).toDouble();
    c.open_C3 = readLine(file).toDouble();
    c.short_Z0 = readLine(file).toDouble();
    c.short_delay = readLine(file).toDouble();
    c.short_loss = readLine(file).toDouble();
    c.short_L0 = readLine(file).toDouble();
    c.short_L1 = readLine(file).toDouble();
    c.short_L2 = readLine(file).toDouble();
    c.short_L3 = readLine(file).toDouble();
    c.load_Z0 = readLine(file).toDouble();
    c.through_Z0 = readLine(file).toDouble();
    c.through_delay = readLine(file).toDouble();
    c.through_loss = readLine(file).toDouble();
    if(c.open_measurements) {
        c.open_file = readLine(file).toStdString();
        c.open_Sparam = readLine(file).toInt();
    }
    if(c.short_measurements) {
        c.short_file = readLine(file).toStdString();
        c.short_Sparam = readLine(file).toInt();
    }
    if(c.load_measurements) {
        c.load_file = readLine(file).toStdString();
        c.load_Sparam = readLine(file).toInt();
    }
    if(c.through_measurements) {
        c.through_file = readLine(file).toStdString();
        c.through_Sparam1 = readLine(file).toInt();
        c.through_Sparam2 = readLine(file).toInt();
    }
    c.TRL_through_Z0 = readLine(file).toDouble();
    c.TRL_reflection_short = readLine(file).toDouble();
    c.TRL_line_delay = readLine(file).toDouble();
    c.TRL_line_minfreq = readLine(file).toDouble();
    c.TRL_line_maxfreq = readLine(file).toDouble();
    file.close();
    return c;
}

void Calkit::edit()
{
    auto dialog = new CalkitDialog(*this);
    dialog->show();
}

Calkit::SOLT Calkit::toSOLT(double frequency)
{
    fillTouchstoneCache();
    SOLT ref;
    if(load_measurements) {
        ref.Load = ts_load->interpolate(frequency).S[0];
    } else {
        auto imp_load = complex<double>(load_Z0, 0);
        ref.Load = (imp_load - complex<double>(50.0)) / (imp_load + complex<double>(50.0));
    }

    if(open_measurements) {
        ref.Open = ts_open->interpolate(frequency).S[0];
    } else {
        // calculate fringing capacitance for open
        double Cfringing = open_C0 * 1e-15 + open_C1 * 1e-27 * frequency + open_C2 * 1e-36 * pow(frequency, 2) + open_C3 * 1e-45 * pow(frequency, 3);
        // convert to impedance
        if (Cfringing == 0) {
            // special case to avoid issues with infinity
            ref.Open = complex<double>(1.0, 0);
        } else {
            auto imp_open = complex<double>(0, -1.0 / (frequency * 2 * M_PI * Cfringing));
            ref.Open = (imp_open - complex<double>(50.0)) / (imp_open + complex<double>(50.0));
        }
        // transform the delay into a phase shift for the given frequency
        double open_phaseshift = -2 * M_PI * frequency * 2 * open_delay * 1e-12;
        double open_att_db = open_loss * 1e9 * 4.3429 * 2 * open_delay * 1e-12 / open_Z0 * sqrt(frequency / 1e9);
        double open_att = pow(10.0, -open_att_db / 10.0);
        auto open_correction = polar<double>(open_att, open_phaseshift);
        ref.Open *= open_correction;
    }

    if(short_measurements) {
        ref.Short = ts_short->interpolate(frequency).S[0];
    } else {
        // calculate inductance for short
        double Lseries = short_L0 * 1e-12 + short_L1 * 1e-24 * frequency + short_L2 * 1e-33 * pow(frequency, 2) + short_L3 * 1e-42 * pow(frequency, 3);
        // convert to impedance
        auto imp_short = complex<double>(0, frequency * 2 * M_PI * Lseries);
        ref.Short =  (imp_short - complex<double>(50.0)) / (imp_short + complex<double>(50.0));
        // transform the delay into a phase shift for the given frequency
        double short_phaseshift = -2 * M_PI * frequency * 2 * short_delay * 1e-12;
        double short_att_db = short_loss * 1e9 * 4.3429 * 2 * short_delay * 1e-12 / short_Z0 * sqrt(frequency / 1e9);;
        double short_att = pow(10.0, -short_att_db / 10.0);
        auto short_correction = polar<double>(short_att, short_phaseshift);
        ref.Short *= short_correction;
    }

    if(through_measurements) {
        auto interp = ts_through->interpolate(frequency);
        ref.ThroughS11 = interp.S[0];
        ref.ThroughS12 = interp.S[1];
        ref.ThroughS21 = interp.S[2];
        ref.ThroughS22 = interp.S[3];
    } else {
        // calculate effect of through
        double through_phaseshift = -2 * M_PI * frequency * through_delay * 1e-12;
        double through_att_db = through_loss * 1e9 * 4.3429 * through_delay * 1e-12 / through_Z0 * sqrt(frequency / 1e9);;
        double through_att = pow(10.0, -through_att_db / 10.0);
        ref.ThroughS12 = polar<double>(through_att, through_phaseshift);
        // Assume symmetric and perfectly matched through for other parameters
        ref.ThroughS21 = ref.ThroughS12;
        ref.ThroughS11 = 0.0;
        ref.ThroughS22 = 0.0;
    }

    return ref;
}

Calkit::TRL Calkit::toTRL(double)
{
    TRL trl;
    // reflection coefficent sign depends on whether an open or short is used
    trl.reflectionIsNegative = TRL_reflection_short;
    // assume ideal through for now
    trl.ThroughS11 = 0.0;
    trl.ThroughS12 = 1.0;
    trl.ThroughS21 = 1.0;
    trl.ThroughS22 = 0.0;
    return trl;
}

double Calkit::minFreq(bool TRL)
{
    if(TRL) {
        return TRL_line_minfreq;
    } else {
        fillTouchstoneCache();
        double min = 0;
        array<Touchstone*, 4> ts_list = {ts_open, ts_short, ts_load, ts_through};
        // find the highest minimum frequency in all measurement files
        for(auto ts : ts_list) {
            if(!ts) {
                // this calibration standard is defined by coefficients, no minimum frequency
                continue;
            }
            if(ts->minFreq() > min) {
                min = ts->minFreq();
            }
        }
        return min;
    }
}

double Calkit::maxFreq(bool TRL)
{
    if(TRL) {
        return TRL_line_maxfreq;
    } else {
        fillTouchstoneCache();
        double max = std::numeric_limits<double>::max();
        array<Touchstone*, 4> ts_list = {ts_open, ts_short, ts_load, ts_through};
        // find the highest minimum frequency in all measurement files
        for(auto ts : ts_list) {
            if(!ts) {
                // this calibration standard is defined by coefficients, no minimum frequency
                continue;
            }
            if(ts->maxFreq() < max) {
                max = ts->maxFreq();
            }
        }
        return max;
    }
}

bool Calkit::isTRLReflectionShort() const
{
    return TRL_reflection_short;
}

void Calkit::clearTouchstoneCache()
{
    delete ts_open;
    ts_open = nullptr;
    delete ts_short;
    ts_short = nullptr;
    delete ts_load;
    ts_load = nullptr;
    delete ts_through;
    ts_through = nullptr;
    ts_cached = false;
}

void Calkit::fillTouchstoneCache()
{
    if(ts_cached) {
        return;
    }
    if(open_measurements) {
        ts_open = new Touchstone(1);
        *ts_open = Touchstone::fromFile(open_file);
        ts_open->reduceTo1Port(open_Sparam);
    }
    if(short_measurements) {
        ts_short = new Touchstone(1);
        *ts_short = Touchstone::fromFile(short_file);
        ts_open->reduceTo1Port(short_Sparam);
    }
    if(load_measurements) {
        ts_load = new Touchstone(1);
        *ts_load = Touchstone::fromFile(load_file);
        ts_open->reduceTo1Port(load_Sparam);
    }
    if(through_measurements) {
        ts_through = new Touchstone(2);
        *ts_through = Touchstone::fromFile(through_file);
        ts_through->reduceTo2Port(through_Sparam1, through_Sparam2);
    }
    ts_cached = true;
}
