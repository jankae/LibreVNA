#ifndef LIBREVNADRIVER_H
#define LIBREVNADRIVER_H

#include "../devicedriver.h"

#include "../../VNA_embedded/Application/Communication/Protocol.hpp"

#include <functional>

class LibreVNADriver : public DeviceDriver
{
    friend class CompoundDriver;
    Q_OBJECT
public:
    enum class TransmissionResult {
        Ack,
        Nack,
        Timeout,
        InternalError,
    };
    Q_ENUM(TransmissionResult)

    LibreVNADriver();

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
    virtual unsigned int getSApoints() {return SApoints;}

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

    /**
     * @brief Registers metatypes within the Qt Framework.
     *
     * If the device driver uses a queued signal/slot connection with custom data types, these types must be registered before emitting the signal.
     * Register them within this function with qRegisterMetaType<Type>("Name");
     */
    virtual void registerTypes() override;

    enum class Synchronization {
        Disabled = 0,
        GUI = 1,
        Reserved = 2,
        ExternalTrigger = 3,
        Last = 4,
    };

    void setSynchronization(Synchronization s, bool master);

public:
signals:
    // Required for the compound device driver
    void passOnReceivedPacket(const Protocol::PacketInfo& packet);
public:
    virtual bool SendPacket(const Protocol::PacketInfo& packet, std::function<void(TransmissionResult)> cb = nullptr, unsigned int timeout = 500) = 0;
    bool sendWithoutPayload(Protocol::PacketType type, std::function<void(TransmissionResult)> cb = nullptr);
    virtual bool updateFirmware(QString file) override;

    unsigned int getMaxAmplitudePoints() const;

    QString getFirmwareMagicString();

    unsigned int getProtocolVersion() const;

signals:
    void receivedAnswer(const LibreVNADriver::TransmissionResult &result);
    void receivedPacket(const Protocol::PacketInfo& packet);

    void receivedTrigger(LibreVNADriver *driver, bool set);

protected slots:
    void handleReceivedPacket(const Protocol::PacketInfo& packet);
protected:
    QString hardwareVersionToString(uint8_t version);

    bool connected;
    unsigned int protocolVersion;
    QString serial;
    Info info;
    uint8_t hardwareVersion;
    unsigned int limits_maxAmplitudePoints;

    Protocol::DeviceStatus lastStatus;

    bool skipOwnPacketHandling;
    bool zerospan;
    unsigned int SApoints;

    Synchronization sync;
    bool syncMaster;

    bool isIdle;
    Protocol::PacketInfo lastNonIdlePacket;

    std::map<int, int> portStageMapping; // maps from excitedPort (count starts at one) to stage (count starts at zero)

    // Driver specific settings
    bool captureRawReceiverValues;
    bool harmonicMixing;
    bool SASignalID;
    bool SAUseDFT;
    double SARBWLimitForDFT;
    bool VNASuppressInvalidPeaks;
    bool VNAAdjustPowerLevel;

    QDialog *manualControlDialog;
};

Q_DECLARE_METATYPE(Protocol::PacketInfo)
Q_DECLARE_METATYPE(LibreVNADriver::TransmissionResult)
Q_DECLARE_METATYPE(Protocol::AmplitudeCorrectionPoint);


#endif // LIBREVNADRIVER_H
