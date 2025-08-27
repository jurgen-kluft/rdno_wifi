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
        WiFiServer  gTcpServer(31337);

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
            return nstatus::Idle; // Unknown status
        }

        bool SetModeStation() { return WiFi.mode(WIFI_STA); }
        bool SetModeAP() { return WiFi.mode(WIFI_AP); }
        bool SetModeAPSTA() { return WiFi.mode(WIFI_AP_STA); }

        // TODO move into rdno_apps
        s16 KeyToIndex(const char* str, s32 len)
        {
            if (len == 4 && strncmp(str, "ssid", 4) == 0)
                return 0;
            if (len == 8 && strncmp(str, "password", 8) == 0)
                return 1;
            if (len == 9 && strncmp(str, "ap_ssid", 7) == 0)
                return 2;
            if (len == 11 && strncmp(str, "ap_password", 11) == 0)
                return 3;
            if (len == 13 && strncmp(str, "remote_server", 13) == 0)
                return 4;
            if (len == 11 && strncmp(str, "remote_port", 11) == 0)
                return 5;

            // check if 'str' is a number and return it as index
            char* endPtr = nullptr;
            long  index  = strtol(str, &endPtr, 10);
            if (endPtr != str && index >= 0 && index < 108)
            {
                return static_cast<s16>(index);
            }
            return -1;
        }

        void UpdateConfig(nvstore::config_t* config, s16 (*funcNameToIndex)(const char* str, s32 len))
        {
            nwifi::Disconnect();
            nwifi::SetModeAP();

            // Start the access point to receive configuration
            WiFi.softAP(config->m_ap_ssid, config->m_ap_password);
            nserial::Println("Access Point started to configure settings.");

            // Start the TCP server to listen for configuration data
            gTcpServer.begin();

            char msgBytes[256];
            s32  msgLength = 0;
            while (true)
            {
                // Wait for a client to connect
                WiFiClient client = gTcpServer.available();
                while (client)
                {
                    // receive message

                    if (msgLength > 0)
                    {
                        const char* msg    = msgBytes;
                        const char* msgEnd = msg + msgLength;
                        const char* outKey;
                        s32         outKeyLength;
                        const char* outValue;
                        s32         outValueLength;
                        while (nvstore::ParseKeyValue(msg, msgEnd, outKey, outKeyLength, outValue, outValueLength))
                        {
                            const s16 index = funcNameToIndex(outKey, outKeyLength);
                            nvstore::ParseValue(config, index, outValue, outValueLength);
                        }

                        msgLength = 0; // reset message length for next message

                        // Check if we have valid WiFi configuration
                        bool valid_wifi_config   = strlen(config->m_ssid) > 0 && strlen(config->m_password) > 0;
                        bool valid_server_config = strlen(config->m_remote_server) > 0 && config->m_remote_port > 0;
                        if (valid_wifi_config && valid_server_config)
                        {
                            break; // exit the loop if we have valid configuration
                        }
                    }
                }
            }

            gTcpServer.stop();           // Stop the TCP server
            WiFi.softAPdisconnect(true); // Stop the access point

            nserial::Println("Configuration access point stopped.");
        }

        bool SetHostname(const char* hostname) { return WiFi.setHostname(hostname); }

        bool ConfigIpAddrNone() { return WiFi.config(INADDR_NONE, INADDR_NONE, INADDR_NONE); }

        nstatus::status_t Begin(const char* ssid)
        {
            // TODO Compile time verification of nstatus::status_t matching the values from the WiFi library ?
            wl_status_t status = WiFi.begin(ssid);
            return sArduinoStatusToNstatus(status);
        }

        nstatus::status_t BeginEncrypted(const char* ssid, const char* passphrase)
        {
            // TODO Compile time verification of nstatus::status_t matching the values from the WiFi library ?
            wl_status_t status = WiFi.begin(ssid, passphrase);
            return sArduinoStatusToNstatus(status);
        }

        void Disconnect() { WiFi.disconnect(); }

        s32 HostByName(const char* hostname, IPAddress_t& outAddr)
        {
            IPAddress addr;
            s32       result     = WiFi.hostByName(hostname, addr);
            outAddr.m_address[0] = addr[0];
            outAddr.m_address[1] = addr[1];
            outAddr.m_address[2] = addr[2];
            outAddr.m_address[3] = addr[3];
            return result;
        }

        IPAddress_t LocalIP()
        {
            IPAddress   ip = WiFi.localIP();
            IPAddress_t ipAddr;
            ipAddr.m_address[0] = ip[0];
            ipAddr.m_address[1] = ip[1];
            ipAddr.m_address[2] = ip[2];
            ipAddr.m_address[3] = ip[3];
            return ipAddr;
        }

        MACAddress_t MacAddress()
        {
            MACAddress_t macAddress;
            uint8_t*     mac = WiFi.macAddress(&macAddress.m_address[0]);
            return macAddress;
        }

        s32  RSSI() { return WiFi.RSSI(); }
        s32  ScanNetworks() { return WiFi.scanNetworks(); }
        void SetDNS(const IPAddress_t& dns)
        {
            IPAddress ip(dns.m_address[0], dns.m_address[1], dns.m_address[2], dns.m_address[3]);
            WiFi.setDNS(ip);
        }

        // TODO Should we add compile time verification of the nencryption::type_t
        // matching the values from the WiFi library?

        nstatus::status_t Status()
        {
            wl_status_t status = WiFi.status();
            return sArduinoStatusToNstatus(status);
        }

        const char* StatusStr(nstatus::status_t status)
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

        bool Reconnect() { return WiFi.reconnect(); }

    } // namespace nwifi
} // namespace ncore

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

        bool SetModeStation() { return true; }
        bool SetModeAP() { return true; }
        bool SetModeAPSTA() { return true; }
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

        void              Disconnect() { CurrentStatus = nstatus::Idle; }
        int               HostByName(const char* hostname, const char* addr) { return 0; }
        IPAddress_t       LocalIP() { return &CurrentLocalIP; }
        MACAddress_t      MacAddress() { return MACAddress_t{0, 0, 0, 0, 0, 0}; }
        int               RSSI() { return CurrentRSSI; }
        int               ScanNetworks() { return CurrentNetworks; }
        void              SetDNS(const IPAddress_t& dns) { CurrentDNS = dns; }
        nstatus::status_t Status() { return CurrentStatus; }
        const char*       StatusStr(nstatus::status_t status) { return "Simulated"; }
        bool              Reconnect() { return true; }

    } // namespace nwifi
} // namespace ncore

#endif
