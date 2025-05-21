#ifndef __RDNO_CORE_WIFI_H__
#define __RDNO_CORE_WIFI_H__
#include "rdno_core/c_target.h"
#ifdef USE_PRAGMA_ONCE
#    pragma once
#endif

#include "rdno_core/c_debug.h"

namespace ncore
{
    namespace nwifi
    {
        // @see: https://www.arduino.cc/en/Reference/WiFi

        // IPAddress ...
        struct IPAddress_t
        {
            byte A, B, C, D;
        };

        struct MACAddress_t
        {
            byte A, B, C, D, E, F;
        };

        struct BSID_t
        {
            byte B0, B1, B2, B3, B4, B5, B6, B7;
        };

        namespace nencryption
        {
            enum enum_t
            {
                TypeAuto = 8,
                TypeCCMP = 4,
                TypeNone = 7,
                TypeTKIP = 2,
                TypeWEP  = 5,
            };
        }

        namespace nstatus
        {
            enum enum_t
            {
                Idle            = 0,
                NoSSIDAvailable = 1,
                ScanCompleted   = 2,
                Connected       = 3,
                ConnectionLost  = 4,
                ConnectFailed   = 5,
                Disconnected    = 6,
                NoShield        = 255,
            };
        }

        // BSSID gets the MAC address of the router you are connected to.
        // @see: https://www.arduino.cc/en/Reference/WiFiBSSID
        BSID_t* GetBSSID();

        // Begin initializes the WiFi library's network settings and provides the current status.
        // @see: https://www.arduino.cc/en/Reference/WiFiBegin
        void Begin(const char* ssid);

        // BeginEncrypted initializes the WiFi library's network settings and provides the current status.
        // @see: https://www.arduino.cc/en/Reference/WiFiBegin
        void BeginEncrypted(const char* ssid, const char* passphrase);

        // Disconnect disconnects the WiFi shield from the current network.
        // @see: https://www.arduino.cc/en/Reference/WiFiDisconnect
        void Disconnect();

        // EncryptionType gets the encryption type of the current network.
        // @see: https://www.arduino.cc/en/Reference/WiFiEncryptionType
        nencryption::enum_t GetEncryptionType();

        // HostByName ...
        int HostByName(const char* hostname, const char* addr);

        // LocalIP gets the WiFi shield's IP address.
        // @see: https://www.arduino.cc/en/Reference/WiFiLocalIP
        IPAddress_t* LocalIP();

        // MacAddress gets the MAC address of the WiFi shield.
        // @see: https://www.arduino.cc/en/Reference/WiFiMacAddress
        MACAddress_t* MacAddress();

        // RSSI gets the signal strength of the connection to the router.
        // @see: https://www.arduino.cc/en/Reference/WiFiRSSI
        int RSSI();

        // ScanNetworks scans for available WiFi networks and returns the discovered number.
        // @see: https://www.arduino.cc/en/Reference/WiFiScanNetworks
        int ScanNetworks();

        // SetDNS allows you to configure the DNS (Domain Name System) server.
        // @see: https://www.arduino.cc/en/Reference/WiFiSetDns
        void SetDNS(const IPAddress_t& dns);

        // Status returns the connection status.
        // @see: https://www.arduino.cc/en/Reference/WiFiStatus
        nstatus::enum_t Status();

        // SSID gets the SSID of the current network.
        // @see: https://www.arduino.cc/en/Reference/WiFiSSID
        const char* SSID();

        // SSID gets the SSID of a scanned network
        // @see: https://www.arduino.cc/en/Reference/WiFiSSID
        const char* SSID(s32 index);

    }  // namespace nwifi
}  // namespace ncore

#endif  // __RDNO_CORE_WIFI_H__
