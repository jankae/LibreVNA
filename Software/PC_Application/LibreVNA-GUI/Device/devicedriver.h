#ifndef DEVICEDRIVER_H
#define DEVICEDRIVER_H

/**
  * This file contains the device interface.
  *
  * To add support for a new hardware device perform the following steps:
  * - Derive from this class
  * - Implement all pure virtual functions
  * - Implement the virtual functions if the device supports the specific function
  * - Add the new driver to getDrivers()
  */

#include "Tools/parameters.h"
#include "savable.h"
#include "scpi.h"

#include <set>
#include <complex>

#include <QObject>
#include <QAction>

class DeviceDriver : public QObject
{
    Q_OBJECT
public:
    DeviceDriver() {}
    virtual ~DeviceDriver();

    /**
     * @brief Returns a list of all available drivers
     * @return driverlist
     */
    static std::vector<DeviceDriver*> getDrivers();

    /**
     * @brief Returns the driver name. It must be unique across all implemented drivers and is used to identify the driver
     * @return driver name
     */
    virtual QString getDriverName() =  0;
    /**
     * @brief Lists all available devices by their serial numbers
     * @return Serial numbers of detected devices
     */
    virtual std::set<QString> GetAvailableDevices() = 0;

protected:
    /**
     * @brief Connects to a device, given by its serial number
     *
     * @param serial Serial number of device that should be connected to
     * @return true if connection successful, otherwise false
     */
    virtual bool connectTo(QString serial) = 0;
    /**
     * @brief Disconnects from device. Has no effect if no device was connected
     */
    virtual void disconnect() = 0;

public:
    /**
     * @brief Returns the serial number of the connected device
     * @return Serial number of connected device (empty string if no device is connected)
     */
    virtual QString getSerial() = 0;

    enum class Feature {
        // VNA features
        VNA,
        VNAFrequencySweep,
        VNAPowerSweep,
        VNAZeroSpan,
        VNALogSweep,
        VNADwellTime,
        // Generator features
        Generator,
        // Spectrum analyzer features
        SA,
        SATrackingGenerator,
        SATrackingOffset,
        // External reference
        ExtRefIn,
        ExtRefOut,
    };

    class Info {
    public:
        Info();
        QString firmware_version;
        QString hardware_version;
        std::set<Feature> supportedFeatures;
        struct {
            struct {
                // Number of VNA ports
                unsigned int ports;
                // Sweep frequency limits in Hz
                double minFreq, maxFreq;
                // IF bandwidth limits in Hz
                double minIFBW, maxIFBW;
                // Maximum number of points per sweep
                unsigned int maxPoints;
                // Stimulus level limits in dBm
                double mindBm, maxdBm;
                // dwell time limit
                double maxDwellTime;
            } VNA;
            struct {
                // Number of ports
                unsigned int ports;
                // Frequency limits in Hz
                double minFreq, maxFreq;
                // Output level limits in dBm
                double mindBm, maxdBm;
            } Generator;
            struct {
                // Number of ports
                unsigned int ports;
                // Sweep frequency limits in Hz
                double minFreq, maxFreq;
                // RBW limits in Hz
                double minRBW, maxRBW;
                // Output level limits  of the tracking generator in dBm
                double mindBm, maxdBm;
            } SA;
        } Limits;
        void subset(const Info &info);
    };

    /**
     * @brief Returns the device information. This function will be called when a device has been connected. Its return value must be valid
     * directly after returning from DeviceDriver::connectTo()
     *
     * Emit the InfoUpdate() signal whenever the return value of this function changes.
     *
     * @return Device information
     */
    virtual Info getInfo() = 0;
signals:
    /**
     * @brief Emit this signal whenever the device info has been updated
     */
    void InfoUpdated();
public:
    /**
     * @brief Checks whether a specific feature is supported
     * @param f Feature to check
     * @return true if feature is supported, false otherwise
     */
    bool supports(Feature f) { return getInfo().supportedFeatures.count(f);}

    /**
     * Status flags
     */
    enum class Flag {
        // The input is overloaded with a signal that is too large
        Overload,
        // A PLL failed to lock
        Unlocked,
        // The requested output amplitude can not be reached
        Unlevel,
        // The external reference input is in use
        ExtRef,
    };

    /**
     * @brief Returns a set of all active flags
     *
     * There is also a convenience function to check a specific flag, see DeviceDriver::asserted()
     *
     * @return Set of active flags
     */
    virtual std::set<Flag> getFlags() = 0;

signals:
    /**
     * @brief Emit this signal whenever a flag changes
     */
    void FlagsUpdated();
public:

    /**
     * @brief Checks whether a specific flag is asserted
     * @param f Flag to check
     * @return true if flag is asserted, false otherwise
     */
    bool asserted(Flag f) {return getFlags().count(f);}

    /**
     * @brief Returns the device status string. It will be displayed in the status bar of the application
     *
     * Emit the StatusUpdated() signal whenever the return value of this function changes
     *
     * @return Status string
     */
    virtual QString getStatus() {return "";}
signals:
    /**
     * @brief Emit this signal whenever the device status has changed (return value of getStatus() has changed)
     */
    void StatusUpdated();
public:
    /**
     * @brief Returns the driver specific settings
     *
     * The settings are returned as a vector of SettingDescriptions, consisting of:
     *      - var: Pointer to the setting variable (should be a private member of the derived class)
     *      - name: Arbitrary string used to persistently store this setting (never visible to the user)
     *      - def: Default value of the setting
     *
     * These settings will be persistent across reboots.
     *
     * @return Map of driver specific settings
     */
    std::vector<Savable::SettingDescription> driverSpecificSettings() {return specificSettings;}

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
    virtual QWidget* createSettingsWidget() {return nullptr;}

    /**
     * @brief Return driver specific actions.
     *
     * The returned actions will be appended to the device menu.
     *
     * @return List of actions
     */
    std::vector<QAction*> driverSpecificActions() {return specificActions;}

    /**
     * @brief Return driver specific SCPI commands
     *
     * The returned commands will be added to the :DEV SCPI node
     *
     * @return List of SCPI commands
     */
    std::vector<SCPICommand*> driverSpecificSCPICommands() {return specificSCPIcommands;}

    /**
     * @brief Return driver specific SCPI nodes
     *
     * The returned nodes (which may contain further nodes/commands) will be added to the :DEV SCPI node
     *
     * @return List of SCPI nodes
     */
    std::vector<SCPINode*> driverSpecificSCPINodes() {return specificSCPInodes;}

    class VNASettings {
    public:
        // Start/stop frequency. Both values will be identical for power sweeps and zero span
        double freqStart, freqStop;
        // Start/stop stimulus level. Both values will be identical for frequency sweeps and zero span
        double dBmStart, dBmStop;
        // IF bandwidth
        double IFBW;
        // Number of points in the sweep
        int points;
        // Logarithmic sweep flag, set if sweep should be logarithmic
        bool logSweep;
        // List of ports that should be excited during the sweep (port count starts at 1)
        std::vector<int> excitedPorts;
        // amount of time the source stays at each point before taking measurements. Ignore if not supported
        double dwellTime;
    };

    class VNAMeasurement {
    public:
        // Number of the point in the sweep (starts at 0)
        unsigned int pointNum;
        // Characteristic impedance of the measurement result (typically 50 Ohm)
        double Z0;
        union {
            struct {
                // for non-zero span
                // Frequency of the point
                double frequency;
                // Stimulus level of the point
                double dBm;
            };
            struct {
                // for zero span
                // time in us since first datapoint
                double us;
            };
        };
        // S parameter measurements
        // Key: S parameter name, e.g. "S11"
        // Value: complex measurement in real/imag (linear, not in dB)
        std::map<QString, std::complex<double>> measurements;

        Sparam toSparam(int ports = 0) const;
        /* Sets the measurement values in the VNAmeasurement (if existent) to the values from the S parameter matrix.
         * The portMapping parameter can be used to specify which values to set from which S parameter:
         * Example: S parameter contains 4 port S parameters, but the VNAmeasurement is 2 port only with this mapping:
         *      VNAMeasurement port | S parameter port
         *      --------------------|-----------------
         *      1                   | 2
         *      2                   | 4
         * This means that we want S22 (from the 4 port S parameter) stored as S11 (in the VNAMeasurement).
         * Function call for this example: fromSparam(S, {2,4})
         *
         * If no portMapping is specified, the port order (and mapping) from the S paramters are kept.
         */
        void fromSparam(Sparam S, std::vector<unsigned int> portMapping = {});
        VNAMeasurement interpolateTo(const VNAMeasurement &to, double a);
    };

    /**
     * @brief Names of available measurements.
     *
     * The names must be identical to the names used in the returned VNAMeasurement.
     * Typically the S parameters, e.g. this function may return {"S11","S12","S21","S22"} but any other names are also allowed.
     *
     * @return List of available VNA measurement parameters
     */
    virtual QStringList availableVNAMeasurements() {return {};}

    /**
     * @brief Configures the VNA and starts a sweep
     * @param s VNA settings
     * @param cb Callback, must be called after the VNA has been configured
     * @return true if configuration successful, false otherwise
     */
    virtual bool setVNA(const VNASettings &s, std::function<void(bool)> cb = nullptr) {Q_UNUSED(s) Q_UNUSED(cb) return false;}
signals:
    /**
     * @brief This signal must be emitted whenever a VNA measurement is complete and should be passed on to the GUI
     * @param m VNA measurement
     */
    void VNAmeasurementReceived(VNAMeasurement m);

public:
    class SASettings {
    public:
        enum class Window {
            None = 0,
            Kaiser = 1,
            Hann = 2,
            FlatTop = 3,
            Last
        };
        enum class Detector {
            PPeak = 0,
            NPeak = 1,
            Sample = 2,
            Normal = 3,
            Average = 4,
            Last
        };
        // Start/stop frequency. Both values will be identical for zero span
        double freqStart, freqStop;
        // Resolution bandwidth
        double RBW;
        // Window type
        Window window;
        // Detector type
        Detector detector;
        // Tracking generator enable
        bool trackingGenerator;
        // Port at which the tracking generator is active. Port count starts at 1
        int trackingPort;
        // Offset frequency of the tracking generator
        double trackingOffset;
        // Output level of the tracking generator
        double trackingPower;
    };
    class SAMeasurement {
    public:
        // Number of point in the sweep
        unsigned int pointNum;
        union {
            struct {
                // for non-zero span
                double frequency;
            };
            struct {
                // for zero span
                double us; // time in us since first datapoint
            };
        };
        // S parameter measurements
        // Key: S parameter name, e.g. "PORT1"
        // Value: measurement in linear voltage (linear, not in dB). A value of 1.0 means 0dBm
        std::map<QString, double> measurements;
    };

    /**
     * @brief Names of available measurements.
     *
     * The names must be identical to the names used in the returned SAMeasurement.
     * Typically the port names, e.g. this function may return {"PORT1","PORT2"} but any other names are also allowed.
     *
     * @return List of available SA measurement parameters
     */
    virtual QStringList availableSAMeasurements() {return {};}
    /**
     * @brief Configures the SA and starts a sweep
     * @param s SA settings
     * @param cb Callback, must be called after the SA has been configured
     * @return true if configuration successful, false otherwise
     */
    virtual bool setSA(const SASettings &s, std::function<void(bool)> cb = nullptr) {Q_UNUSED(s) Q_UNUSED(cb) return false;}

    /**
     * @brief Returns the number of points in one spectrum analyzer sweep (as configured by the last setSA() call)
     * @return Number of points in the sweep
     */
    virtual unsigned int getSApoints() {return 0;}
signals:
    /**
     * @brief This signal must be emitted whenever a SA measurement is complete and should be passed on to the GUI
     * @param m SA measurement
     */
    void SAmeasurementReceived(SAMeasurement m);

public:
    class SGSettings {
    public:
        // Output frequency
        double freq;
        // Output signal level
        double dBm;
        // Output port. Port count starts at 1, set to zero to disable all ports
        int port;
    };

    /**
     * @brief Names of available generator ports.
     *
     * Typically the port names, e.g. this function may return {"PORT1","PORT2"} but any other names are also allowed.
     *
     * @return List of available SA measurement parameters
     */
    virtual QStringList availableSGPorts() {return {};}
    /**
     * @brief Configures the generator
     * @param s Generator settings
     * @return true if configuration successful, false otherwise
     */
    virtual bool setSG(const SGSettings &s) {Q_UNUSED(s) return false;}

    /**
     * @brief Sets the device to idle
     *
     * Stops all sweeps and signal generation
     *
     * @param cb Callback, must be called after the device has stopped all operations
     * @return true if configuration successful, false otherwise
     */
    virtual bool setIdle(std::function<void(bool)> cb = nullptr) {Q_UNUSED(cb) return false;}

    /**
     * @brief Returns the available options for the external reference input
     * @return External reference input options
     */
    virtual QStringList availableExtRefInSettings() {return {};}

    /**
     * @brief Returns the available options for the external reference output
     * @return External reference output options
     */
    virtual QStringList availableExtRefOutSettings() {return {};}

    /**
     * @brief Configures the external reference input/output
     * @param option_in Reference input option (one of the options returned by availableExtRefInSettings())
     * @param option_out Reference output option (one of the options returned by availableExtRefOutSettings())
     * @return true if configuration successful, false otherwise
     */
    virtual bool setExtRef(QString option_in, QString option_out) {Q_UNUSED(option_in) Q_UNUSED(option_out) return false;}

    /**
     * @brief maximumSupportedPorts Maximum number of supported ports by the GUI. No device driver may report a higher number of ports than this value
     */
    static constexpr unsigned int maximumSupportedPorts = 8;

    static Info getInfo(DeviceDriver* driver) {
        if(driver) {
            return driver->getInfo();
        } else {
            return Info();
        }
    }

    /**
     * @brief Registers metatypes within the Qt Framework.
     *
     * If the device driver uses a queued signal/slot connection with custom data types, these types must be registered before emitting the signal.
     * Register them within this function with qRegisterMetaType<Type>("Name");
     */
    virtual void registerTypes() {}

signals:
    /**
     * @brief Emit this signal when the device connection has been lost unexpectedly
     *
     * The device driver should do nothing else, the disconnect() function will be called from the application after this signal has been emitted.
     */
    void ConnectionLost();
    /**
     * @brief Emit this signal whenever a debug log line from the device has been received. May be left unused if device does not support debug output
     * @param line
     */
    void LogLineReceived(QString line);

    /**
     * @brief Emit this signal whenever the device driver wants complete control over the device. Once emitted, no other part of the application
     * will try to communicate until releaseControl() is emitted
     */
    void acquireControl();

    /**
     * @brief Emit this signal whenever the device driver wants to return control to the application.
     */
    void releaseControl();

    /**
     * @brief Emit this to temporarily add a new SCPI command to the root node.
     *
     * Before deleting the command, removeSCPICommand must be emitted.
     * When the device is disconnected, all added commands will be automatically removed.
     *
     * @param cmd Command to add
     */
    void addSCPICommand(SCPICommand *cmd);

    /**
     * @brief Emit this to remove a temporarily added SCPI command.
     *
     * @param cmd Command to remove
     */
    void removeSCPICommand(SCPICommand *cmd);

    /**
     * @brief Emit this to temporarily add a new SCPI node to the root node.
     *
     * Before deleting the node, removeSCPINode must be emitted.
     * When the device is disconnected, all added nodes will be automatically removed.
     *
     * @param node Node to add
     */
    void addSCPINode(SCPINode *node);

    /**
     * @brief Emit this to remove a temporarily added SCPI node.
     *
     * @param node Node to remove
     */
    void removeSCPINode(SCPINode *node);


public:
    bool connectDevice(QString serial, bool isIndepedentDriver = false);
    void disconnectDevice();
    virtual bool updateFirmware(QString file) {Q_UNUSED(file) return false;}
    static DeviceDriver* getActiveDriver() {return activeDriver;}
    static unsigned int SApoints();

protected:
    // Each driver implementation may add specific actionsm, settings or commands. All of these must
    // be created in the constructor and added to the following vectors:

    // A list of actions specific to the driver. They will show up in the device menu
    std::vector<QAction*> specificActions;

    // A list of settings specific to the driver. They will be stored/recalled as part of the preferences.
    // If a setting should be user-changeable in the preferences, createSettingsWidget() must include some
    // widget to modify that setting
    std::vector<Savable::SettingDescription> specificSettings;

    // A list of SCPI commands. They will be available at the root node whenever the device driver is in use.
    // Avoid name collisions with commands/nodes already implemented in appwindow.cpp.
    // Use this for commands that will be available whenever the device is connected. For commands that are
    // not always available, use the addSCPICommand and removeSCPICommand signals.
    std::vector<SCPICommand*> specificSCPIcommands;

    // A list of SCPI nodes. They will be available at the root node whenever the device driver is in use.
    // Avoid name collisions with commands/nodes already implemented in appwindow.cpp
    // Use this for nodes that will be available whenever the device is connected. For nodes that are
    // not always available, use the addSCPINode and removeSCPINode signals.
    std::vector<SCPINode*> specificSCPInodes;

private:
    static DeviceDriver *activeDriver;
};

Q_DECLARE_METATYPE(DeviceDriver::VNAMeasurement)
Q_DECLARE_METATYPE(DeviceDriver::SAMeasurement)

#endif // VIRTUALDEVICE_H
