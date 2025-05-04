#ifndef COMPOUNDDRIVER_H
#define COMPOUNDDRIVER_H

#include "../../devicedriver.h"
#include "compounddevice.h"

#include <QMutex>

class CompoundDriver : public DeviceDriver
{
public:
    CompoundDriver();
    ~CompoundDriver();

    /**
     * @brief Returns the driver name. It must be unique across all implemented drivers and is used to identify the driver
     * @return driver name
     */
    virtual QString getDriverName() override {return "LibreVNA/Compound";}
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
    virtual bool connectTo(QString getSerial) override;
    /**
     * @brief Disconnects from device. Has no effect if no device was connected
     */
    virtual void disconnect() override;

public:
    /**
     * @brief Returns the serial number of the connected device
     * @return Serial number of connected device (empty string if no device is connected)
     */
    virtual QString getSerial() override;

    /**
     * @brief Returns the device information. This function will be called when a device has been connected. Its return value must be valid
     * directly after returning from DeviceDriver::connectTo()
     *
     * Emit the InfoUpdate() signal whenever the return value of this function changes.
     *
     * @return Device information
     */
    virtual Info getInfo() override {return info;}

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
     * @brief Returns a widget to edit the driver specific settings.
     *
     * The widget is displayed in the global settings dialog and allows the user to edit the settings
     * specific to this driver. The application takes ownership of the widget after returning,
     * create a new widget for every call to this function. If the driver has no specific settings
     * or the settings do not need to be editable by the user, return a nullptr. In this case, no
     * page for this driver is created in the settings dialog
     * @return newly constructed settings widget or nullptr
     */
    virtual QWidget* createSettingsWidget() override;

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
     * @brief Names of available measurements.
     *
     * The names must be identical to the names used in the returned SAMeasurement.
     * Typically the port names, e.g. this function may return {"PORT1","PORT2"} but any other names are also allowed.
     *
     * @return List of available SA measurement parameters
     */
    virtual QStringList availableSAMeasurements() override;
    /**
     * @brief Configures the SA and starts a sweep
     * @param s SA settings
     * @param cb Callback, must be called after the SA has been configured
     * @return true if configuration successful, false otherwise
     */
    virtual bool setSA(const SASettings &s, std::function<void(bool)> cb = nullptr) override;

    /**
     * @brief Returns the number of points in one spectrum analyzer sweep (as configured by the last setSA() call)
     * @return Number of points in the sweep
     */
    virtual unsigned int getSApoints() override {return SApoints;}

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

    static std::set<QString> getIndividualDeviceSerials();

private slots:
    void triggerReceived(LibreVNADriver *device, bool set);
private:
    void parseCompoundJSON();
    void createCompoundJSON();
    void incomingPacket(LibreVNADriver *device, const Protocol::PacketInfo &p);
    void updatedInfo(LibreVNADriver *device);
    void updatedStatus(LibreVNADriver *device, const Protocol::DeviceStatus &status);
    void datapointReceivecd(LibreVNADriver *dev, Protocol::VNADatapoint<32> *data);
    void spectrumResultReceived(LibreVNADriver *dev, Protocol::SpectrumAnalyzerResult res);
    void enableTriggerForwarding();
    void disableTriggerForwarding();

    Info info;
    std::map<LibreVNADriver*, Info> deviceInfos;
    std::map<LibreVNADriver*, Protocol::DeviceStatus> deviceStatus;
    std::map<int, std::map<LibreVNADriver*, Protocol::VNADatapoint<32>*>> compoundVNABuffer;
    std::map<int, std::map<LibreVNADriver*, Protocol::SpectrumAnalyzerResult>> compoundSABuffer;
    Protocol::DeviceStatus lastStatus;

    // Parsed configuration of compound devices (as extracted from compoundJSONString
    std::vector<CompoundDevice*> configuredDevices;

    std::map<int, int> portStageMapping; // maps from excitedPort (count starts at one) to stage (count starts at zero)

    // All possible drivers to interact with a LibreVNA
    std::vector<LibreVNADriver*> drivers;

    // Configuration of the device we are connected to
    CompoundDevice activeDevice;
    bool connected;
    bool triggerForwarding;
    QMutex triggerMutex;
    std::vector<LibreVNADriver*> devices;
    bool zerospan;
    bool isIdle;
    enum class Types{VNA, SA, SG};
    struct {
        VNASettings vna;
        SASettings sa;
        SGSettings sg;
        Types type;
    } lastNonIdleSettings;
    unsigned int VNApoints;
    unsigned int SApoints;

    // Driver specific settings
    bool captureRawReceiverValues;
    bool preservePhase;
    QString compoundJSONString;

    // Buffers for storing individual device answers
    std::map<LibreVNADriver*, bool> results;
    void checkIfAllTransmissionsComplete(std::function<void(bool)> cb = nullptr);
};

#endif // COMPOUNDDRIVER_H
