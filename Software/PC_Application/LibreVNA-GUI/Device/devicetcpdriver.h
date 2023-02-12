#ifndef DEVICETCPDRIVER_H
#define DEVICETCPDRIVER_H

#include "devicedriver.h"

#include <QHostAddress>

/**
 * @brief DeviceTCPDriver
 *
 * This is a convenience class for device drivers that are TCP/IP based and need to search for
 * connected devices at specific IP addresses. If this is the case for your device driver,
 * inherit from DeviceTCPDriver instead of DeviceDriver.
 * When searching for connected devices, search the IP addresses as given by getSearchAddresses().
 * When overwriting createSettingsWidget in your derived driver, include the widget returned
 * by DeviceTCPDriver::createSettingsWidget if you want the search addresses to be editable by
 * the user.
 */

class DeviceTCPDriver : public DeviceDriver
{
public:
    DeviceTCPDriver(QString driverName);

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
    virtual QWidget *createSettingsWidget() override;

protected:
    std::vector<QHostAddress> getSearchAddresses();
private:
    void parseSearchAddressString();
    void updateSearchAddressString();
    std::vector<QHostAddress> searchAddresses;
    QString searchAddressString;
};

#endif // DEVICETCPDRIVER_H
