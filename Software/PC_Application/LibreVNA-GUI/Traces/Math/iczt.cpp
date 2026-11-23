#include "iczt.h"

#include "Traces/fftcomplex.h"
#include "Util/util.h"
#include "appwindow.h"

#include <chrono>
#include <thread>
#include <cmath>
#include <algorithm>
#include <numeric>

#include <QVBoxLayout>
#include <QLabel>
#include <QDebug>
#include <QDialog>
#include <QComboBox>
#include <QCheckBox>
#include <QRadioButton>
#include <QSpinBox>
#include <QDialogButtonBox>

using namespace Math;
using namespace std;

static constexpr double C0 = 299792458.0;
static constexpr double M_PI = 3.14159265358979323846;

ICZT::ICZT()
{
    automaticDC = true;
    manualDC = 1.0;
    mode = Mode::LowpassStep;
    velocityFactor = 0.66;
    gammaClip = 0.98;

    destructing = false;
    thread = new ICZTThread(*this);
    thread->start(ICZTThread::Priority::LowestPriority);

    connect(&window, &WindowFunction::changed, this, &ICZT::updateICZT);
}

ICZT::~ICZT()
{
    // tell thread to exit
    destructing = true;
    semphr.release();
    thread->wait();
    delete thread;
}

TraceMath::DataType ICZT::outputType(TraceMath::DataType inputType)
{
    if(inputType == DataType::Frequency) {
        return DataType::Time;
    } else {
        return DataType::Invalid;
    }
}

QString ICZT::description()
{
    QString ret = "ICZT (";
    switch(mode) {
        case Mode::LowpassStep:
            ret += "lowpass-step for impedance";
            break;
        case Mode::LowpassImpulse:
            ret += "lowpass-impulse";
            break;
        case Mode::BandpassImpulse:
            ret += "bandpass-impulse";
            break;
    }
    ret += "), window: " + window.getDescription();
    ret += ", vf: " + QString::number(velocityFactor, 'f', 2);
    return ret;
}

void ICZT::edit()
{
    auto d = new QDialog();
    auto layout = new QVBoxLayout();
    
    // Mode selection
    auto modeLabel = new QLabel("Mode:");
    auto modeBox = new QComboBox();
    modeBox->addItem("Lowpass Step (Impedance)");
    modeBox->addItem("Lowpass Impulse");
    modeBox->addItem("Bandpass Impulse");
    modeBox->setCurrentIndex((int)mode);
    
    layout->addWidget(modeLabel);
    layout->addWidget(modeBox);
    
    // Window selection
    auto windowWidget = window.createEditor();
    layout->addWidget(new QLabel("Window Function:"));
    layout->addWidget(windowWidget);
    
    // Velocity factor
    auto vfLabel = new QLabel("Velocity Factor:");
    auto vfEdit = new QSpinBox();
    vfEdit->setMinimum(10);
    vfEdit->setMaximum(100);
    vfEdit->setValue((int)(velocityFactor * 100));
    vfEdit->setSuffix("%");
    layout->addWidget(vfLabel);
    layout->addWidget(vfEdit);
    
    // DC handling (only for lowpass modes)
    auto dcLabel = new QLabel("DC Estimation:");
    auto dcAutomatic = new QRadioButton("Automatic");
    auto dcManual = new QRadioButton("Manual");
    dcAutomatic->setChecked(automaticDC);
    dcManual->setChecked(!automaticDC);
    
    layout->addWidget(dcLabel);
    layout->addWidget(dcAutomatic);
    layout->addWidget(dcManual);
    
    // Buttons
    auto buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
    layout->addWidget(buttonBox);
    
    d->setLayout(layout);
    
    // Connect signals
    connect(modeBox, QOverload<int>::of(&QComboBox::currentIndexChanged), this, [=](int idx) {
        mode = (Mode)idx;
        updateICZT();
    });
    
    connect(vfEdit, QOverload<int>::of(&QSpinBox::valueChanged), this, [=](int val) {
        velocityFactor = val / 100.0;
        updateICZT();
    });
    
    connect(dcAutomatic, &QRadioButton::toggled, this, [=](bool checked) {
        automaticDC = checked;
        updateICZT();
    });
    
    connect(buttonBox, &QDialogButtonBox::accepted, d, &QDialog::accept);
    connect(buttonBox, &QDialogButtonBox::rejected, d, &QDialog::reject);
    
    if(AppWindow::showGUI()) {
        d->show();
    }
}

QWidget *ICZT::createExplanationWidget()
{
    auto w = new QWidget();
    auto label = new QLabel(
        "ICZT (Inverse Chirp-Z Transform) TDR\n\n"
        "Provides high-precision impedance measurement using direct inverse transform.\n\n"
        "Modes:\n"
        "• Lowpass Step: Best for impedance display\n"
        "• Lowpass Impulse: Experimental impulse response\n"
        "• Bandpass Impulse: Good for locating reflections\n\n"
        "Special lowpass windowing preserves DC and low frequencies "
        "while tapering only the high-frequency end."
    );
    label->setWordWrap(true);
    auto layout = new QVBoxLayout();
    layout->addWidget(label);
    w->setLayout(layout);
    return w;
}

nlohmann::json ICZT::toJSON()
{
    nlohmann::json j;
    j["mode"] = static_cast<int>(mode);
    j["window"] = window.toJSON();
    j["velocity_factor"] = velocityFactor;
    j["automatic_dc"] = automaticDC;
    if(!automaticDC) {
        j["manual_dc_real"] = manualDC.real();
        j["manual_dc_imag"] = manualDC.imag();
    }
    j["gamma_clip"] = gammaClip;
    return j;
}

void ICZT::fromJSON(nlohmann::json j)
{
    if(j.contains("mode")) {
        mode = (Mode)j["mode"].get<int>();
    }
    if(j.contains("window")) {
        window.fromJSON(j["window"]);
    }
    velocityFactor = j.value("velocity_factor", 0.66);
    automaticDC = j.value("automatic_dc", true);
    if(!automaticDC && j.contains("manual_dc_real")) {
        manualDC = complex<double>(j["manual_dc_real"].get<double>(), j["manual_dc_imag"].get<double>());
    }
    gammaClip = j.value("gamma_clip", 0.98);
}

void ICZT::setMode(Mode m)
{
    if(mode == m) return;
    mode = m;
    if(input) {
        inputSamplesChanged(0, input->numSamples());
    }
}

Mode ICZT::getMode() const
{
    return mode;
}

const WindowFunction& ICZT::getWindow() const
{
    return window;
}

double ICZT::getVelocityFactor() const
{
    return velocityFactor;
}

void ICZT::setVelocityFactor(double vf)
{
    if(velocityFactor != vf) {
        velocityFactor = vf;
        updateICZT();
    }
}

void ICZT::setAutomaticDC(bool auto_dc)
{
    if(automaticDC != auto_dc) {
        automaticDC = auto_dc;
        updateICZT();
    }
}

void ICZT::setManualDC(complex<double> dc)
{
    if(manualDC != dc) {
        manualDC = dc;
        if(!automaticDC) {
            updateICZT();
        }
    }
}

void ICZT::setGammaClip(double clip)
{
    if(gammaClip != clip) {
        gammaClip = clip;
        if(mode == Mode::LowpassStep) {
            updateICZT();
        }
    }
}

void ICZT::inputSamplesChanged(unsigned int begin, unsigned int end)
{
    Q_UNUSED(begin);
    Q_UNUSED(end);
    if(input && input->numSamples() >= 2) {
        semphr.release();
        success();
    } else {
        clearOutput();
        warning("Not enough input samples");
    }
}

void ICZT::updateICZT()
{
    if(input && input->numSamples() >= 2) {
        inputSamplesChanged(0, input->numSamples());
    }
}

void ICZT::clearOutput()
{
    dataMutex.lock();
    data.clear();
    impedanceData.clear();
    dataMutex.unlock();
    emit outputSamplesChanged(0, 0);
}

double ICZT::checkLinearFrequency(const vector<double>& freq_hz)
{
    if(freq_hz.size() < 2) {
        throw runtime_error("Need at least 2 frequency points");
    }
    
    vector<double> df(freq_hz.size() - 1);
    for(size_t i = 0; i < freq_hz.size() - 1; i++) {
        df[i] = freq_hz[i+1] - freq_hz[i];
    }
    
    double df_median = df[df.size() / 2];
    if(df_median <= 0) {
        throw runtime_error("Frequency axis must be strictly increasing");
    }
    
    double max_err = 0;
    for(double d : df) {
        max_err = max(max_err, abs(d - df_median));
    }
    
    double allowed = max(abs(df_median) * 1e-5, 1e-6);
    if(max_err > allowed) {
        qWarning() << "Warning: Frequency spacing not perfectly linear, error:" << max_err;
    }
    
    return df_median;
}

complex<double> ICZT::estimateDC(const vector<complex<double>>& s11)
{
    if(s11.empty()) {
        return 1.0;
    }
    return s11.front();
}

vector<double> ICZT::makeLowpassWindow(size_t n)
{
    // For lowpass mode: preserve low frequencies, taper high frequencies
    // This is achieved by using the second half of a full-sized window
    
    vector<double> w(n);
    
    switch(window.getType()) {
        case WindowFunction::Type::Kaiser: {
            // Create double-size Kaiser window, take second half
            vector<double> full(2 * n);
            double beta = window.getBeta();
            for(size_t i = 0; i < 2 * n; i++) {
                double x = (double)i / (2 * n - 1);
                // Bessel function I0
                double arg = beta * sqrt(1.0 - (2.0*x - 1.0)*(2.0*x - 1.0));
                double i0_arg = 1.0;
                for(int k = 1; k <= 20; k++) {
                    i0_arg += pow(arg / 2.0 / k, 2);
                }
                full[i] = i0_arg / i0_arg; // Normalized
            }
            copy(full.begin() + n, full.end(), w.begin());
            break;
        }
        case WindowFunction::Type::Hann: {
            for(size_t i = 0; i < n; i++) {
                double x = (double)(i + n) / (2 * n);
                w[i] = 0.5 * (1.0 - cos(2.0 * M_PI * x));
            }
            break;
        }
        case WindowFunction::Type::Hamming: {
            for(size_t i = 0; i < n; i++) {
                double x = (double)(i + n) / (2 * n);
                w[i] = 0.54 - 0.46 * cos(2.0 * M_PI * x);
            }
            break;
        }
        default:
            fill(w.begin(), w.end(), 1.0);
    }
    
    return w;
}

vector<complex<double>> ICZT::calculateBandpassImpulse(
    const vector<double>& freq_hz,
    const vector<complex<double>>& s11,
    const vector<double>& t_s)
{
    vector<double> w = makeLowpassWindow(freq_hz.size());
    
    // Apply window
    vector<complex<double>> weighted_s11(s11.size());
    double scale = 0;
    for(size_t i = 0; i < s11.size(); i++) {
        weighted_s11[i] = s11[i] * w[i];
        scale += w[i];
    }
    
    if(scale <= 0) {
        throw runtime_error("Window scale is zero");
    }
    
    vector<complex<double>> y(t_s.size());
    const size_t block_size = 256;
    
    for(size_t start = 0; start < t_s.size(); start += block_size) {
        size_t stop = min(start + block_size, t_s.size());
        
        for(size_t t_idx = start; t_idx < stop; t_idx++) {
            complex<double> sum = 0;
            for(size_t f_idx = 0; f_idx < freq_hz.size(); f_idx++) {
                double phase = 2.0 * M_PI * t_s[t_idx] * freq_hz[f_idx];
                complex<double> kernel = exp(complex<double>(0, phase));
                sum += kernel * weighted_s11[f_idx];
            }
            y[t_idx] = sum / scale;
        }
    }
    
    return y;
}

vector<complex<double>> ICZT::calculateLowpassImpulse(
    const vector<double>& freq_hz,
    const vector<complex<double>>& s11,
    const vector<double>& t_s)
{
    // Use only positive frequencies
    vector<double> f;
    vector<complex<double>> s;
    
    for(size_t i = 0; i < freq_hz.size(); i++) {
        if(freq_hz[i] > 0) {
            f.push_back(freq_hz[i]);
            s.push_back(s11[i]);
        }
    }
    
    if(f.empty()) {
        throw runtime_error("Need positive frequency points for lowpass mode");
    }
    
    double df = checkLinearFrequency(freq_hz);
    complex<double> sdc = estimateDC(s11);
    
    vector<double> w = makeLowpassWindow(f.size());
    
    vector<complex<double>> y(t_s.size());
    const size_t block_size = 256;
    
    for(size_t start = 0; start < t_s.size(); start += block_size) {
        size_t stop = min(start + block_size, t_s.size());
        
        for(size_t t_idx = start; t_idx < stop; t_idx++) {
            complex<double> sum = sdc;
            for(size_t f_idx = 0; f_idx < f.size(); f_idx++) {
                double phase = 2.0 * M_PI * t_s[t_idx] * f[f_idx];
                complex<double> kernel = exp(complex<double>(0, phase));
                sum += 2.0 * real(kernel * (s[f_idx] * w[f_idx])) * df;
            }
            y[t_idx] = sum;
        }
    }
    
    return y;
}

vector<complex<double>> ICZT::calculateLowpassStep(
    const vector<double>& freq_hz,
    const vector<complex<double>>& s11,
    const vector<double>& t_s)
{
    // Use only positive frequencies
    vector<double> f;
    vector<complex<double>> s;
    
    for(size_t i = 0; i < freq_hz.size(); i++) {
        if(freq_hz[i] > 0) {
            f.push_back(freq_hz[i]);
            s.push_back(s11[i]);
        }
    }
    
    if(f.empty()) {
        throw runtime_error("Need positive frequency points for lowpass step mode");
    }
    
    double df = checkLinearFrequency(freq_hz);
    complex<double> sdc = estimateDC(s11);
    
    vector<double> w = makeLowpassWindow(f.size());
    
    // Calculate coefficients: s * w * df / (j*2*pi*f)
    vector<complex<double>> coeff(f.size());
    for(size_t i = 0; i < f.size(); i++) {
        coeff[i] = s[i] * w[i] * df / (complex<double>(0, 2.0 * M_PI * f[i]));
    }
    
    vector<complex<double>> y(t_s.size());
    const size_t block_size = 256;
    
    for(size_t start = 0; start < t_s.size(); start += block_size) {
        size_t stop = min(start + block_size, t_s.size());
        
        for(size_t t_idx = start; t_idx < stop; t_idx++) {
            complex<double> gamma = 0.5 * sdc;
            for(size_t f_idx = 0; f_idx < f.size(); f_idx++) {
                double phase = 2.0 * M_PI * t_s[t_idx] * f[f_idx];
                complex<double> kernel = exp(complex<double>(0, phase));
                gamma += 2.0 * real(kernel * coeff[f_idx]);
            }
            y[t_idx] = gamma;
        }
    }
    
    return y;
}

// Thread implementation
ICZTThread::ICZTThread(ICZT &iczt)
    : iczt(iczt)
{
}

void ICZTThread::run()
{
    qDebug() << "ICZT thread starting";
    using namespace std::chrono;
    auto lastCalc = system_clock::now();
    
    while(1) {
        iczt.semphr.acquire();
        iczt.semphr.tryAcquire(iczt.semphr.available());
        
        if(iczt.destructing) {
            qDebug() << "ICZT thread exiting";
            return;
        }
        
        if(!iczt.input) {
            continue;
        }
        
        auto inputData = iczt.input->getData();
        if(inputData.empty()) {
            iczt.clearOutput();
            iczt.warning("Not enough input samples");
            continue;
        }
        
        try {
            // Extract frequency and S11 data
            vector<double> freq_hz(inputData.size());
            vector<complex<double>> s11(inputData.size());
            
            for(size_t i = 0; i < inputData.size(); i++) {
                freq_hz[i] = inputData[i].x;
                s11[i] = inputData[i].y;
            }
            
            // Determine time span
            double freq_span = freq_hz.back() - freq_hz.front();
            double time_max = 1.0 / freq_span;
            
            // Generate output time points
            size_t num_points = min((size_t)1000, inputData.size() * 2);
            vector<double> t_s(num_points);
            for(size_t i = 0; i < num_points; i++) {
                t_s[i] = (double)i / (num_points - 1) * time_max;
            }
            
            // Calculate TDR
            vector<complex<double>> tdr_result;
            
            switch(iczt.mode) {
                case ICZT::Mode::BandpassImpulse:
                    tdr_result = iczt.calculateBandpassImpulse(freq_hz, s11, t_s);
                    break;
                case ICZT::Mode::LowpassImpulse:
                    tdr_result = iczt.calculateLowpassImpulse(freq_hz, s11, t_s);
                    break;
                case ICZT::Mode::LowpassStep:
                    tdr_result = iczt.calculateLowpassStep(freq_hz, s11, t_s);
                    break;
            }
            
            // Store results
            iczt.dataMutex.lock();
            iczt.data.resize(tdr_result.size());
            iczt.impedanceData.resize(tdr_result.size());
            
            for(size_t i = 0; i < tdr_result.size(); i++) {
                iczt.data[i].x = t_s[i];
                iczt.data[i].y = tdr_result[i];
                
                // Calculate impedance for lowpass-step mode
                if(iczt.mode == ICZT::Mode::LowpassStep) {
                    double g = real(tdr_result[i]);
                    g = max(-iczt.gammaClip, min(iczt.gammaClip, g));
                    // Z = Z0 * (1 + gamma) / (1 - gamma)
                    double denom = 1.0 - g;
                    if(abs(denom) > 1e-10) {
                        iczt.impedanceData[i] = 50.0 * (1.0 + g) / denom;
                    } else {
                        iczt.impedanceData[i] = 50.0;
                    }
                } else {
                    iczt.impedanceData[i] = 50.0;
                }
            }
            
            size_t result_size = iczt.data.size();
            iczt.dataMutex.unlock();
            
            iczt.success();
            emit iczt.outputSamplesChanged(0, result_size);
            
        } catch(const exception& e) {
            qWarning() << "ICZT calculation error:" << e.what();
            iczt.clearOutput();
            iczt.error(QString::fromStdString(e.what()));
            continue;
        }
        
        // Rate limiting
        this_thread::sleep_until(lastCalc + duration<double>(0.05));
        lastCalc = system_clock::now();
    }
}
