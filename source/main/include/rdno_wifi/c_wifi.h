#ifndef __RDNO_CORE_WIFI_H__
#define __RDNO_CORE_WIFI_H__
#include "rdno_core/c_target.h"
#ifdef USE_PRAGMA_ONCE
#    pragma once
#endif

#include "rdno_core/c_network.h"
#include "rdno_wifi/c_definitions.h"

namespace ncore
{
    namespace nwifi
    {
        // @see: https://www.arduino.cc/en/Reference/WiFi

        // SetModexxx sets the WiFi mode to station (client), access point, or both.
        bool set_mode_AP();      // Set the WiFi mode to access point mode
        bool set_mode_STA();     // Set the WiFi mode to station (client) mode
        bool set_mode_AP_STA();  // Set the WiFi mode to both station and access point mode

        // FastConnect tries to connect using saved WiFi settings.
        bool fast_connect(nconfig::config_t* config);

        // Begin initializes the WiFi library's network settings and provides the current status.
        // @see: https://www.arduino.cc/en/Reference/WiFiBegin
        nstatus::status_t begin(const char* ssid);
        nstatus::status_t begin_AP(const char* ap_ssid, const char* ap_password);

        // BeginEncrypted initializes the WiFi library's network settings and provides the current status.
        // @see: https://www.arduino.cc/en/Reference/WiFiBegin
        nstatus::status_t begin_encrypted(const char* ssid, const char* passphrase);

        // Status returns the connection status.
        // @see: https://www.arduino.cc/en/Reference/WiFiStatus
        nstatus::status_t status();
        const char*       status_str(nstatus::status_t status);

        // Disconnect disconnects the WiFi shield from the current network.
        // @see: https://www.arduino.cc/en/Reference/WiFiDisconnect
        void disconnect();
        void disconnect_AP(bool wifioff);

        // LocalIP gets the WiFi shield's IP address.
        // @see: https://www.arduino.cc/en/Reference/WiFiLocalIP
        IPAddress_t local_IP();

        // MacAddress gets the MAC address of the WiFi shield.
        // @see: https://www.arduino.cc/en/Reference/WiFiMacAddress
        MACAddress_t mac_address();

        // RSSI gets the signal strength of the connection to the router.
        // @see: https://www.arduino.cc/en/Reference/WiFiRSSI
        s32 get_RSSI();

        // SetDNS allows you to configure the DNS (Domain Name System) server.
        // @see: https://www.arduino.cc/en/Reference/WiFiSetDns
        void set_DNS(const IPAddress_t& dns);

        // Reconnect attempts to reconnect to the last connected network.
        // Basically does a Disconnect() followed by a Begin().
        bool reconnect();

    }  // namespace nwifi
}  // namespace ncore

#endif  // __RDNO_CORE_WIFI_H__
