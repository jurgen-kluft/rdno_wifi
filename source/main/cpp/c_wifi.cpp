#ifdef TARGET_ESP32

#    include "Arduino.h"

// Generated
#    include "WiFi.h"
#    include "WiFiUdp.h"

#    include "rdno_wifi/c_wifi.h"

namespace ncore
{
    namespace nwifi
    {
        IPAddress_t gLocalIP;

        BSID_t* GetBSSID() { return (BSID_t*)WiFi.BSSID(); }
        bool    SetHostname(const char* hostname)
        {
            return WiFi.setHostname(hostname);
        }

        bool ConfigIpAddrNone() { return WiFi.config(INADDR_NONE, INADDR_NONE, INADDR_NONE); }

        nstatus::status_t Begin(const char* ssid)
        {
            // TODO Compile time verification of nstatus::status_t matching the values from the WiFi library ?
            nstatus::status_t status = (nstatus::status_t)WiFi.begin(ssid);
            return status;
        }

        nstatus::status_t BeginEncrypted(const char* ssid, const char* passphrase)
        {
            // TODO Compile time verification of nstatus::status_t matching the values from the WiFi library ?
            nstatus::status_t status = (nstatus::status_t)WiFi.begin(ssid, passphrase);
            return status;
        }

        void Disconnect() { WiFi.disconnect(); }

        int HostByName(const char* hostname, IPAddress_t& outAddr)
        {
            IPAddress addr;
            int       result = WiFi.hostByName(hostname, addr);
            outAddr          = *((IPAddress_t*)&addr);
            return result;
        }

        IPAddress_t* LocalIP()
        {
            IPAddress ip = WiFi.localIP();
            gLocalIP     = *((IPAddress_t*)&ip);
            return &gLocalIP;
        }

        int  RSSI() { return WiFi.RSSI(); }
        int  ScanNetworks() { return WiFi.scanNetworks(); }
        void SetDNS(const IPAddress_t& dns)
        {
            IPAddress ip = *((IPAddress*)&dns);
            WiFi.setDNS(ip);
        }

        // TODO Should we add compile time verification of the nencryption::type_t
        // matching the values from the WiFi library?

        nstatus::status_t Status() { return (nstatus::status_t)(WiFi.status()); }
        bool              Reconnect() { WiFi.reconnect(); }
        const char*       SSID() { return WiFi.SSID().c_str(); }
        const char*       SSID(s32 index)
        {
            String ssid = WiFi.SSID(index);
            return ssid.c_str();
        }

    }  // namespace nwifi
}  // namespace ncore

#else

#    include "rdno_wifi/c_wifi.h"

namespace ncore
{
    namespace nwifi
    {
        // WiFi simulation

        const s32 MaxSocketNum = 4096;

        BSID_t              CurrentBSSID              = {0, 0, 0, 0, 0, 0, 0, 0};
        IPAddress_t         CurrentDNS                = {0, 0, 0, 0};
        nencryption::type_t EncryptionType            = nencryption::None;
        IPAddress_t         CurrentGateway            = {127, 0, 0, 255};
        IPAddress_t         CurrentLocalIP            = {127, 0, 0, 1};
        s32                 CurrentNetworks           = 0;
        s32                 CurrentRSSI               = -1;
        const char*         CurrentSSID               = "";
        nstatus::status_t   CurrentStatus             = nstatus::Idle;
        int                 SocketPort[MaxSocketNum]  = {0};
        int                 SocketState[MaxSocketNum] = {0};

        BSID_t* GetBSSID() { return &CurrentBSSID; }

        bool SetHostname(const char* hostname) { return true; }
        bool ConfigIpAddrNone() { return true; }

        nstatus::status_t Begin(const char* ssid)
        {
            CurrentRSSI   = 0;
            CurrentSSID   = ssid;
            CurrentStatus = nstatus::Connected;
            return CurrentStatus;
        }

        nstatus::status_t BeginEncrypted(const char* ssid, const char* passphrase)
        {
            CurrentRSSI   = 0;
            CurrentSSID   = ssid;
            CurrentStatus = nstatus::Connected;
            return CurrentStatus;
        }

        void                Disconnect() { CurrentStatus = nstatus::Idle; }
        int                 HostByName(const char* hostname, const char* addr) { return 0; }
        IPAddress_t*        LocalIP() { return &CurrentLocalIP; }
        int                 RSSI() { return CurrentRSSI; }
        int                 ScanNetworks() { return CurrentNetworks; }
        void                SetDNS(const IPAddress_t& dns) { CurrentDNS = dns; }
        nstatus::status_t   Status() { return CurrentStatus; }
        bool                Reconnect() { return true; }
        const char*         SSID() { return CurrentSSID; }

    }  // namespace nwifi
}  // namespace ncore

#endif
