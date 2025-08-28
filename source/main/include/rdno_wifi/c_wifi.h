#ifndef __RDNO_CORE_WIFI_H__
#define __RDNO_CORE_WIFI_H__
#include "rdno_core/c_target.h"
#ifdef USE_PRAGMA_ONCE
#    pragma once
#endif

#include "rdno_core/c_network.h"

namespace ncore
{
    namespace nvstore
    {
        struct config_t;
    }

    namespace nencryption
    {
        typedef u8 type_t;

        const type_t None                    = 0;           // Authenticate mode : None
        const type_t OPEN                    = 0;           // Authenticate mode : open
        const type_t WEP                     = 1;           // Authenticate mode : WEP
        const type_t WPA_PSK                 = 2;           // Authenticate mode : WPA_PSK
        const type_t WPA2_PSK                = 3;           // Authenticate mode : WPA2_PSK
        const type_t WPA_WPA2_PSK            = 4;           // Authenticate mode : WPA_WPA2_PSK
        const type_t ENTERPRISE              = 5;           // Authenticate mode : Wi-Fi EAP security
        const type_t WPA2_ENTERPRISE         = ENTERPRISE;  // Authenticate mode : Wi-Fi EAP security
        const type_t WPA3_PSK                = 7;           // Authenticate mode : WPA3_PSK
        const type_t WPA2_WPA3_PSK           = 8;           // Authenticate mode : WPA2_WPA3_PSK
        const type_t WAPI_PSK                = 9;           // Authenticate mode : WAPI_PSK
        const type_t OWE                     = 10;          // Authenticate mode : OWE
        const type_t WPA3_ENT_192            = 11;          // Authenticate mode : WPA3_ENT_SUITE_B_192_BIT
        const type_t WPA3_EXT_PSK            = 12;          // This authentication mode will yield same result as WIFI_AUTH_WPA3_PSK and not recommended to be used. It will be deprecated in future, please use WIFI_AUTH_WPA3_PSK instead.
        const type_t WPA3_EXT_PSK_MIXED_MODE = 13;          // This authentication mode will yield same result as WIFI_AUTH_WPA3_PSK and not recommended to be used. It will be deprecated in future, please use WIFI_AUTH_WPA3_PSK instead.
        const type_t DPP                     = 14;          // Authenticate mode : DPP
        const type_t WPA3_ENTERPRISE         = 15;          // Authenticate mode : WPA3-Enterprise Only Mode
        const type_t WPA2_WPA3_ENTERPRISE    = 16;          // Authenticate mode : WPA3-Enterprise Transition Mode
        const type_t MAX                     = 17;          // Authenticate mode : Maximum value of authentication mode
    }  // namespace nencryption

    namespace nstatus
    {
        typedef s32    status_t;
        const status_t Idle            = 0;
        const status_t NoSSIDAvailable = 1;
        const status_t ScanCompleted   = 2;
        const status_t Connected       = 3;
        const status_t ConnectFailed   = 4;
        const status_t ConnectionLost  = 5;
        const status_t Disconnected    = 6;
        const status_t Stopped         = 254;
        const status_t NoShield        = 255;
    }  // namespace nstatus

    namespace nwifi
    {
        // @see: https://www.arduino.cc/en/Reference/WiFi

        struct BSID_t
        {
            byte B0, B1, B2, B3, B4, B5, B6, B7;
        };

        // SetModexxx sets the WiFi mode to station (client), access point, or both.
        bool SetModeStation();  // Set the WiFi mode to station (client) mode
        bool SetModeAP();       // Set the WiFi mode to access point mode
        bool SetModeAPSTA();    // Set the WiFi mode to both station and access point mode

        // SetHostName ...
        bool SetHostname(const char* hostname);

        // ConfigIpAddrNone sets the IP address to INADDR_NONE.
        bool ConfigIpAddrNone();

        // Begin initializes the WiFi library's network settings and provides the current status.
        // @see: https://www.arduino.cc/en/Reference/WiFiBegin
        nstatus::status_t Begin(const char* ssid);

        // BeginEncrypted initializes the WiFi library's network settings and provides the current status.
        // @see: https://www.arduino.cc/en/Reference/WiFiBegin
        nstatus::status_t BeginEncrypted(const char* ssid, const char* passphrase);

        // Status returns the connection status.
        // @see: https://www.arduino.cc/en/Reference/WiFiStatus
        nstatus::status_t Status();
        const char*       StatusStr(nstatus::status_t status);

          // Disconnect disconnects the WiFi shield from the current network.
          // @see: https://www.arduino.cc/en/Reference/WiFiDisconnect
          void Disconnect();

        // LocalIP gets the WiFi shield's IP address.
        // @see: https://www.arduino.cc/en/Reference/WiFiLocalIP
        IPAddress_t LocalIP();

        // MacAddress gets the MAC address of the WiFi shield.
        // @see: https://www.arduino.cc/en/Reference/WiFiMacAddress
        MACAddress_t MacAddress();

        // RSSI gets the signal strength of the connection to the router.
        // @see: https://www.arduino.cc/en/Reference/WiFiRSSI
        s32 RSSI();

        // ScanNetworks scans for available WiFi networks and returns the discovered number.
        // @see: https://www.arduino.cc/en/Reference/WiFiScanNetworks
        s32 ScanNetworks();

        // SetDNS allows you to configure the DNS (Domain Name System) server.
        // @see: https://www.arduino.cc/en/Reference/WiFiSetDns
        void SetDNS(const IPAddress_t& dns);

        // Reconnect attempts to reconnect to the last connected network.
        // Basically does a Disconnect() followed by a Begin().
        bool Reconnect();

    }  // namespace nwifi
}  // namespace ncore

#endif  // __RDNO_CORE_WIFI_H__
