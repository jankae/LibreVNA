#!/bin/bash -ex

cp -r /usr/local/Frameworks/QtDbus.framework LibreVNA-GUI.app/Contents/Frameworks
cp /usr/local/lib/libusb-1.0.0.dylib LibreVNA-GUI.app/Contents/Frameworks
install_name_tool -change /usr/local/opt/libusb/lib/libusb-1.0.0.dylib @executable_path/../Frameworks/libusb-1.0.0.dylib LibreVNA-GUI.app/Contents/MacOS/LibreVNA-GUI
install_name_tool -change /usr/local/opt/qt/lib/QtWidgets.framework/Versions/A/QtWidgets @executable_path/../Frameworks/QtWidgets.framework/Versions/A/QtWidgets LibreVNA-GUI.app/Contents/MacOS/LibreVNA-GUI
install_name_tool -change /usr/local/opt/qt/lib/QtGui.framework/Versions/A/QtGui @executable_path/../Frameworks/QtGui.framework/Versions/A/QtGui LibreVNA-GUI.app/Contents/MacOS/LibreVNA-GUI
install_name_tool -change /usr/local/opt/qt/lib/QtNetwork.framework/Versions/A/QtNetwork @executable_path/../Frameworks/QtNetwork.framework/Versions/A/QtNetwork LibreVNA-GUI.app/Contents/MacOS/LibreVNA-GUI
install_name_tool -change /usr/local/opt/qt/lib/QtCore.framework/Versions/A/QtCore @executable_path/../Frameworks/QtCore.framework/Versions/A/QtCore LibreVNA-GUI.app/Contents/MacOS/LibreVNA-GUI
cp /usr/local/opt/dbus/lib/libdbus-1.3.dylib LibreVNA-GUI.app/Contents/Frameworks/QtDBus.framework
install_name_tool -change /usr/local/opt/dbus/lib/libdbus-1.3.dylib @executable_path/../Frameworks/QtDbus.framework/libdbus-1.3.dylib LibreVNA-GUI.app/Contents/Frameworks/QtDBus.framework/Versions/A/QtDBus
