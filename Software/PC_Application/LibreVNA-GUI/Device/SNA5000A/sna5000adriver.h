#ifndef SNA5000ADRIVER_H
#define SNA5000ADRIVER_H

#include "../devicetcpdriver.h"

#include "../tracedifferencegenerator.h"

#include <QHostAddress>
#include <QTcpSocket>
#include <QThread>

class SNA5000ADriver : public DeviceTCPDriver
{
    Q_OBJECT
public:
    SNA5000ADriver();
    virtual ~SNA5000ADriver();

    /**
     * @brief Returns the driver name. It must be unique across all implemented drivers and is used to identify the driver
     * @return driver name
     */
    virtual QString getDriverName() override {return "SNA5000A";}
    /**
     * @brief Lists all available devices by their serial numbers
     * @return Serial numbers of detected devices
     */
    virtual std::set<QString> GetAvailableDevices() override;

protected:
    /**
     * @brief Connects to a device, given by its serial number
     *
     * @param serial Serial number of device that should be connected to
     * @return true if connection successful, otherwise false
     */
    virtual bool connectTo(QString serial) override;
    /**
     * @brief Disconnects from device. Has no effect if no device was connected
     */
    virtual void disconnect() override;

public:
    /**
     * @brief Returns the serial number of the connected device
     * @return Serial number of connected device (empty string if no device is connected)
     */
    virtual QString getSerial() override {return serial;}

    /**
     * @brief Returns the device information. This function will be called when a device has been connected. Its return value must be valid
     * directly after returning from DeviceDriver::connectTo()
     *
     * Emit the InfoUpdate() signal whenever the return value of this function changes.
     *
     * @return Device information
     */
    virtual Info getInfo() override;

    /**
     * @brief Returns a set of all active flags
     *
     * There is also a convenience function to check a specific flag, see DeviceDriver::asserted()
     *
     * @return Set of active flags
     */
    virtual std::set<Flag> getFlags() override;

    /**
     * @brief Returns the device status string. It will be displayed in the status bar of the application
     *
     * Emit the StatusUpdated() signal whenever the return value of this function changes
     *
     * @return Status string
     */
    virtual QString getStatus() override;

    /**
     * @brief Names of available measurements.
     *
     * The names must be identical to the names used in the returned VNAMeasurement.
     * Typically the S parameters, e.g. this function may return {"S11","S12","S21","S22"} but any other names are also allowed.
     *
     * @return List of available VNA measurement parameters
     */
    virtual QStringList availableVNAMeasurements() override;

    /**
     * @brief Configures the VNA and starts a sweep
     * @param s VNA settings
     * @param cb Callback, must be called after the VNA has been configured
     * @return true if configuration successful, false otherwise
     */
    virtual bool setVNA(const VNASettings &s, std::function<void(bool)> cb = nullptr) override;

    /**
     * @brief Names of available generator ports.
     *
     * Typically the port names, e.g. this function may return {"PORT1","PORT2"} but any other names are also allowed.
     *
     * @return List of available SA measurement parameters
     */
    virtual QStringList availableSGPorts() override;
    /**
     * @brief Configures the generator
     * @param s Generator settings
     * @return true if configuration successful, false otherwise
     */
    virtual bool setSG(const SGSettings &s) override;

    /**
     * @brief Sets the device to idle
     *
     * Stops all sweeps and signal generation
     *
     * @param cb Callback, must be called after the device has stopped all operations
     * @return true if configuration successful, false otherwise
     */
    virtual bool setIdle(std::function<void(bool)> cb = nullptr) override;

    /**
     * @brief Returns the available options for the external reference input
     * @return External reference input options
     */
    virtual QStringList availableExtRefInSettings() override;

    /**
     * @brief Returns the available options for the external reference output
     * @return External reference output options
     */
    virtual QStringList availableExtRefOutSettings() override;

    /**
     * @brief Configures the external reference input/output
     * @param option_in Reference input option (one of the options returned by availableExtRefInSettings())
     * @param option_out Reference output option (one of the options returned by availableExtRefOutSettings())
     * @return true if configuration successful, false otherwise
     */
    virtual bool setExtRef(QString option_in, QString option_out) override;

private slots:
    void extractTracePoints();
signals:
private slots:
    void handleIncomingData();
private:
    struct {
        bool enabled;
        unsigned int state;
        std::vector<double> xaxis;
        std::map<QString, std::vector<double>> data;
        bool waitingForResponse;
    } traceReader;
    bool traceReaderStop(unsigned int timeout = 1000);
    void traceReaderRestart();
    void traceReaderStatemachine();
    bool waitForLine(unsigned int timeout);
    void write(QString s);
    QString query(QString s, unsigned int timeout = 100);
    long long queryInt(QString s);
    std::vector<double> queryDoubleList(QString s);
    QString serial;
    QTcpSocket dataSocket;

    bool connected;
    Info info;

    std::vector<int> excitedPorts;
    double excitationPower;

    class VNAPoint {
    public:
        unsigned int index;
        double frequency;
        std::map<QString, std::complex<double>> data;
        bool operator==(const VNAPoint& rhs) {
            if(index != rhs.index || frequency != rhs.frequency || data.size() != rhs.data.size()) {
                return false;
            }
            if(data.size() == 0) {
                return true;
            } else {
                return std::prev(data.end())->second == std::prev(rhs.data.end())->second;
            }
//            return index == rhs.index && frequency == rhs.frequency && data.size() == rhs.data.size() && std::equal(data.begin(), data.end(), rhs.data.begin());
        }
    };

    TraceDifferenceGenerator<VNAPoint> *diffGen;

    std::map<QString, QHostAddress> detectedDevices;
};


#endif // SNA5000ADRIVER_H
