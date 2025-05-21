#ifdef TARGET_ESP32

#include "Arduino.h"

// Generated
#include "WiFi.h"
#include "WiFiUdp.h"

#include "rdno_wifi/c_wifi.h"

namespace ncore
{
    namespace nwifi
    {
        IPAddress_t gLocalIP;

        BSID_t* GetBSSID()
        {
            return (BSID_t*)WiFi.BSSID();
        }

        void Begin(const char* ssid)
        {
            WiFi.begin(ssid);
        }

        void BeginEncrypted(const char* ssid, const char* passphrase)
        {
            WiFi.begin(ssid, passphrase);
        }

        void                Disconnect()
        {
            WiFi.disconnect();
        }

        // TODO Should we add compile time verification of the nencryption::enum_t 
        // matching the values from the WiFi library?

        nencryption::enum_t GetEncryptionType()
        {
            //return static_cast<nencryption::enum_t>(WiFi.encryptionType());
            // TODO, no encryptionType() in the ESP32 WiFi library to get the encryption type
            //       of the current network.
            return nencryption::TypeNone;
        }

        int                 HostByName(const char* hostname, IPAddress_t& outAddr)
        {
            IPAddress addr;
            int      result = WiFi.hostByName(hostname, addr);
            outAddr         = *((IPAddress_t*)&addr);
            return result;
        }

        IPAddress_t*        LocalIP()
        {
            IPAddress ip = WiFi.localIP();
            gLocalIP = *((IPAddress_t*)&ip);
            return &gLocalIP;
        }

        int                 RSSI() 
        {
            return WiFi.RSSI();
        }

        int                 ScanNetworks()
        {
            return WiFi.scanNetworks();
        }

        void                SetDNS(const IPAddress_t& dns)
        {
            IPAddress ip = *((IPAddress*)&dns);
            WiFi.setDNS(ip);
        }

        // TODO Should we add compile time verification of the nencryption::enum_t 
        // matching the values from the WiFi library?

        nstatus::enum_t     Status() 
        {
            return static_cast<nstatus::enum_t>(WiFi.status());
        }

        const char*         SSID()
        {
            return WiFi.SSID().c_str();
        }

        const char*         SSID(s32 index)
        {
            String ssid = WiFi.SSID(index);
            return ssid.c_str();
        }

    }  // namespace nwifi
}  // namespace ncore

#else

#include "rdno_wifi/c_wifi.h"

namespace ncore
{
    namespace nwifi
    {
        // WiFi simulation

        const s32 MaxSocketNum = 4096;

        BSID_t              CurrentBSSID              = {0, 0, 0, 0, 0, 0, 0, 0};
        IPAddress_t         CurrentDNS                = {0, 0, 0, 0};
        nencryption::enum_t EncryptionType            = nencryption::TypeNone;
        IPAddress_t         CurrentGateway            = {127, 0, 0, 255};
        IPAddress_t         CurrentLocalIP            = {127, 0, 0, 1};
        s32                 CurrentNetworks           = 0;
        s32                 CurrentRSSI               = -1;
        const char*         CurrentSSID               = "";
        nstatus::enum_t     CurrentStatus             = nstatus::Idle;
        int                 SocketPort[MaxSocketNum]  = {0};
        int                 SocketState[MaxSocketNum] = {0};

        BSID_t* GetBSSID()
        {
            return &CurrentBSSID;
        }

        void Begin(const char* ssid)
        {
            CurrentRSSI   = 0;
            CurrentSSID   = ssid;
            CurrentStatus = nstatus::Connected;
        }

        void BeginEncrypted(const char* ssid, const char* passphrase)
        {
            CurrentRSSI   = 0;
            CurrentSSID   = ssid;
            CurrentStatus = nstatus::Connected;
        }

        void                Disconnect() { CurrentStatus = nstatus::Idle; }
        nencryption::enum_t GetEncryptionType() { return EncryptionType; }
        int                 HostByName(const char* hostname, const char* addr) { return 0; }
        IPAddress_t*        LocalIP() { return &CurrentLocalIP; }
        int                 RSSI() { return CurrentRSSI; }
        int                 ScanNetworks() { return CurrentNetworks; }
        void                SetDNS(const IPAddress_t& dns) { CurrentDNS = dns; }
        nstatus::enum_t     Status() { return CurrentStatus; }
        const char*         SSID() { return CurrentSSID; }

    }  // namespace nwifi
}  // namespace ncore

#endif
