#ifdef TARGET_ARDUINO

#    include "Arduino.h"
#    include "WiFi.h"

#    ifdef TARGET_ESP8266
#        include "ESP8266WiFi.h"
#    endif

#    include "rdno_wifi/c_wifi.h"
#    include "rdno_core/c_config.h"
#    include "rdno_core/c_str.h"

namespace ncore
{
    namespace nwifi
    {
        state_wifi_t gWiFiState;
        void set_state(state_t* state)
        {
            state->wifi = &gWiFiState;
        }

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

        nstatus::status_t begin(const char* ssid)
        {
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
            wl_status_t status = WiFi.begin(ssid, passphrase);
            return sArduinoStatusToNstatus(status);
        }

        void disconnect() { WiFi.disconnect(); }
        void disconnect_AP(bool wifioff) { WiFi.softAPdisconnect(wifioff); }

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

        // ----------------------------------------------------------------------------------------
        // ----------------------------------------------------------------------------------------
        // from: https://github.com/softplus/esp8266-wifi-timing

        // do a fast-connect, if we can, return true if ok
        bool fast_connect_fast_connect(const char* ssid, const char* auth, nconfig::wifi_t* config)
        {
            if (config->ip_address == 0 && config->wifi_channel == 0)
                return false;

            WiFi.persistent(true);
            WiFi.mode(WIFI_STA);
            WiFi.config(config->ip_address, config->ip_gateway, config->ip_mask);

            WiFi.begin(ssid, auth, config->wifi_channel, config->wifi_bssid, true);

            const u32 timeout = millis() + 5000;  // max 5s
            while ((WiFi.status() != WL_CONNECTED) && (millis() < timeout))
            {
                delay(5);
            }

            return (WiFi.status() == WL_CONNECTED);
        }

        // do a normal wifi connection, once connected cache connection info, return true if ok
        bool fast_connect_normal_connect(const char* ssid, const char* auth)
        {
            WiFi.persistent(true);
            WiFi.mode(WIFI_STA);

            WiFi.begin(ssid, auth, 0, NULL, true);

            const u32 timeout = millis() + 15000;  // max 15s
            while ((WiFi.status() != WL_CONNECTED) && (millis() < timeout))
            {
                delay(5);
            }

            if (WiFi.status() == WL_CONNECTED)
                return true;
            return false;
        }

        // Connect to wifi as specified, returns true if ok
        bool fast_connect(nconfig::config_t* config)
        {
            WiFi.setAutoReconnect(false);  // prevent early autoconnect

            str_t wifi_ssid;
            nconfig::get_string(config, nconfig::PARAM_ID_WIFI_SSID, wifi_ssid);

            str_t wifi_auth;
            nconfig::get_string(config, nconfig::PARAM_ID_WIFI_PASSWORD, wifi_auth);

            if (!fast_connect_fast_connect(wifi_ssid.m_const, wifi_auth.m_const, &config->m_wifi))
            {
                if (!fast_connect_normal_connect(wifi_ssid.m_const, wifi_auth.m_const))
                    return false;
            }

            nconfig::wifi_t& w = config->m_wifi;
            w.ip_address       = WiFi.localIP();
            w.ip_gateway       = WiFi.gatewayIP();
            w.ip_mask          = WiFi.subnetMask();
            w.ip_dns1          = WiFi.dnsIP(0);
            w.ip_dns2          = WiFi.dnsIP(1);
            memcpy(w.wifi_bssid, WiFi.BSSID(), 6);
            w.wifi_channel = WiFi.channel();
            return true;
        }

    }  // namespace nwifi
}  // namespace ncore

#else

#    include "rdno_wifi/c_wifi.h"

namespace ncore
{
    namespace nwifi
    {
        // WiFi mock

        const s32 MaxSocketNum = 4096;

        BSID_t              CurrentBSSID              = {0, 0, 0, 0, 0, 0, 0, 0};
        IPAddress_t         CurrentDNS                = {0, 0, 0, 0};
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
        IPAddress_t       local_IP() { return CurrentLocalIP; }
        MACAddress_t      mac_address() { return MACAddress_t{0, 0, 0, 0, 0, 0}; }
        int               get_RSSI() { return CurrentRSSI; }
        void              set_DNS(const IPAddress_t& dns) { CurrentDNS = dns; }
        nstatus::status_t status() { return CurrentStatus; }
        const char*       status_str(nstatus::status_t status) { return "Simulated"; }
        bool              reconnect() { return true; }

    }  // namespace nwifi
}  // namespace ncore

#endif
