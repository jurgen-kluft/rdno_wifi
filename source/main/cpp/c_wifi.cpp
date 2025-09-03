#ifdef TARGET_ESP32

#    include "Arduino.h"
#    include "WiFi.h"

#    include "rdno_wifi/c_wifi.h"
#    include "rdno_core/c_nvstore.h"

namespace ncore
{
    namespace nwifi
    {
        IPAddress_t gLocalIP;

        static inline nstatus::status_t sArduinoStatusToNstatus(s32 status)
        {
            switch (status)
            {
                case WL_NO_SHIELD: return nstatus::NoShield;
                case WL_IDLE_STATUS: return nstatus::Idle;
                case WL_NO_SSID_AVAIL: return nstatus::NoSSIDAvailable;
                case WL_SCAN_COMPLETED: return nstatus::ScanCompleted;
                case WL_CONNECTED: return nstatus::Connected;
                case WL_CONNECT_FAILED: return nstatus::ConnectFailed;
                case WL_CONNECTION_LOST: return nstatus::ConnectionLost;
                case WL_DISCONNECTED: return nstatus::Disconnected;
            }
            return nstatus::Idle;  // Unknown status
        }

        bool set_mode_AP() { return WiFi.mode(WIFI_AP); }
        bool set_mode_STA() { return WiFi.mode(WIFI_STA); }
        bool set_mode_AP_STA() { return WiFi.mode(WIFI_AP_STA); }

        bool set_host_name(const char* hostname) { return WiFi.setHostname(hostname); }

        bool config_IP_AddrNone() { return WiFi.config(INADDR_NONE, INADDR_NONE, INADDR_NONE); }

        nstatus::status_t begin(const char* ssid)
        {
            // TODO Compile time verification of nstatus::status_t matching the values from the WiFi library ?
            wl_status_t status = WiFi.begin(ssid);
            return sArduinoStatusToNstatus(status);
        }

        nstatus::status_t begin_AP(const char* ap_ssid, const char* ap_password)
        {
            const bool result = WiFi.softAP(ap_ssid, ap_password);
            return result ? nstatus::Connected : nstatus::ConnectFailed;
        }

        nstatus::status_t begin_encrypted(const char* ssid, const char* passphrase)
        {
            // TODO Compile time verification of nstatus::status_t matching the values from the WiFi library ?
            wl_status_t status = WiFi.begin(ssid, passphrase);
            return sArduinoStatusToNstatus(status);
        }

        void disconnect() { WiFi.disconnect(); }
        void disconnect_AP(bool wifioff) { WiFi.softAPdisconnect(wifioff); }

        s32 host_by_name(const char* hostname, IPAddress_t& outAddr)
        {
            IPAddress addr;
            s32       result     = WiFi.hostByName(hostname, addr);
            outAddr.m_address[0] = addr[0];
            outAddr.m_address[1] = addr[1];
            outAddr.m_address[2] = addr[2];
            outAddr.m_address[3] = addr[3];
            return result;
        }

        IPAddress_t local_IP()
        {
            IPAddress   ip = WiFi.localIP();
            IPAddress_t ipAddr;
            ipAddr.m_address[0] = ip[0];
            ipAddr.m_address[1] = ip[1];
            ipAddr.m_address[2] = ip[2];
            ipAddr.m_address[3] = ip[3];
            return ipAddr;
        }

        MACAddress_t mac_address()
        {
            MACAddress_t macAddress;
            uint8_t*     mac = WiFi.macAddress(&macAddress.m_address[0]);
            return macAddress;
        }

        s32  get_RSSI() { return WiFi.RSSI(); }
        s32  scan_networks() { return WiFi.scanNetworks(); }
        void set_DNS(const IPAddress_t& dns)
        {
            IPAddress ip(dns.m_address[0], dns.m_address[1], dns.m_address[2], dns.m_address[3]);
            WiFi.setDNS(ip);
        }

        // TODO Should we add compile time verification of the nencryption::type_t
        // matching the values from the WiFi library?

        nstatus::status_t status()
        {
            wl_status_t status = WiFi.status();
            return sArduinoStatusToNstatus(status);
        }

        const char* status_str(nstatus::status_t status)
        {
            switch (status)
            {
                case nstatus::Idle: return "Idle";
                case nstatus::NoShield: return "No WiFi module";
                case nstatus::ScanCompleted: return "Scan completed";
                case nstatus::NoSSIDAvailable: return "Network not available";
                case nstatus::Connected: return "Connected";
                case nstatus::ConnectFailed: return "Connection failed (auth failed?)";
                case nstatus::ConnectionLost: return "Connection lost";
                case nstatus::Disconnected: return "Disconnected";
                case nstatus::Stopped: return "Stopped";
            }
            return "Unknown";
        }

        bool reconnect() { return WiFi.reconnect(); }

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

        bool set_mode_AP() { return true; }
        bool set_mode_STA() { return true; }
        bool set_mode_AP_STA() { return true; }
        bool set_host_name(const char* hostname) { return true; }
        bool config_IP_AddrNone() { return true; }

        nstatus::status_t begin(const char* ssid)
        {
            CurrentRSSI   = 0;
            CurrentSSID   = ssid;
            CurrentStatus = nstatus::Connected;
            return CurrentStatus;
        }

        nstatus::status_t begin_encrypted(const char* ssid, const char* passphrase)
        {
            CurrentRSSI   = 0;
            CurrentSSID   = ssid;
            CurrentStatus = nstatus::Connected;
            return CurrentStatus;
        }

        void              disconnect() { CurrentStatus = nstatus::Idle; }
        int               hostbyname(const char* hostname, const char* addr) { return 0; }
        IPAddress_t       local_IP() { return CurrentLocalIP; }
        MACAddress_t      mac_address() { return MACAddress_t{0, 0, 0, 0, 0, 0}; }
        int               get_RSSI() { return CurrentRSSI; }
        int               scan_networks() { return CurrentNetworks; }
        void              set_DNS(const IPAddress_t& dns) { CurrentDNS = dns; }
        nstatus::status_t status() { return CurrentStatus; }
        const char*       status_str(nstatus::status_t status) { return "Simulated"; }
        bool              reconnect() { return true; }

    }  // namespace nwifi
}  // namespace ncore

#endif
