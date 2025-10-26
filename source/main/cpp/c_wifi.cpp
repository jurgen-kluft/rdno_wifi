#ifdef TARGET_ARDUINO

#    include "Arduino.h"
#    include "WiFi.h"

#    ifdef TARGET_ESP8266
#        include "ESP8266WiFi.h"
#    endif

#    include "rdno_wifi/c_wifi.h"
#    include "rdno_core/c_eeprom.h"
#    include "rdno_core/c_memory.h"
#    include "rdno_core/c_network.h"
#    include "rdno_core/c_serial.h"
#    include "rdno_core/c_str.h"

namespace ncore
{
    namespace nwifi
    {
        state_wifi_t gWiFiState;

        void init_state(state_t* state)
        {
            state->wifi = &gWiFiState;
            g_memclr(&state->wifi->m_cache, sizeof(nwifi::cache_t));
            state->wifi->m_status = nstatus::Disconnected;

            ncore::nserial::println("loading WiFi cache from EEPROM");
            ncore::neeprom::load((byte*)&state->wifi->m_cache, sizeof(ncore::nwifi::cache_t));
            {
                const u32 crc              = state->wifi->m_cache.m_crc;
                state->wifi->m_cache.m_crc = 0;
                if (crc != neeprom::crc32((const byte*)&state->wifi->m_cache, sizeof(nwifi::cache_t)))
                {
                    ncore::nserial::println(" WiFi cache in EEPROM is corrupted (CRC mismatch)");
                    g_memclr(&state->wifi->m_cache, sizeof(nwifi::cache_t));
                } else {
                    ncore::nserial::println("WiFi cache loaded from EEPROM");
                }
                state->wifi->m_cache.m_crc = crc;
            }
        }

        bool set_mode_AP() { return WiFi.mode(WIFI_AP); }
        bool set_mode_STA() { return WiFi.mode(WIFI_STA); }
        bool set_mode_AP_STA() { return WiFi.mode(WIFI_AP_STA); }

        bool begin_AP(const char* ap_ssid, const char* ap_password) { return WiFi.softAP(ap_ssid, ap_password); }

        wl_status_t begin_encrypted(const char* ssid, const char* passphrase) { return WiFi.begin(ssid, passphrase); }

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

        s32 get_RSSI() { return WiFi.RSSI(); }

        void set_DNS(const IPAddress_t& dns)
        {
            IPAddress ip(dns.m_address[0], dns.m_address[1], dns.m_address[2], dns.m_address[3]);
            WiFi.setDNS(ip);
        }

        // TODO Should we add compile time verification of the nencryption::type_t
        // matching the values from the WiFi library?

        // ----------------------------------------------------------------------------------------
        // ----------------------------------------------------------------------------------------
        // from: https://github.com/softplus/esp8266-wifi-timing

        // do a fast-connect, if we can, return true if ok
        void fast_connect_fast(const char* ssid, const char* auth, nwifi::cache_t const& cache)
        {
            WiFi.persistent(true);
            WiFi.mode(WIFI_STA);
            WiFi.config(cache.ip_address, cache.ip_gateway, cache.ip_mask);
            WiFi.begin(ssid, auth, cache.wifi_channel, cache.wifi_bssid, true);
        }

        // do a normal wifi connection, once connected cache connection info, return true if ok
        void fast_connect_normal(const char* ssid, const char* auth)
        {
            WiFi.persistent(true);
            WiFi.mode(WIFI_STA);
            WiFi.begin(ssid, auth, 0, NULL, true);
        }

        // Connect to wifi as specified, returns true if ok
        void connect(state_t* state)
        {
            WiFi.setAutoReconnect(false);  // prevent early autoconnect

            if (state->wifi->m_cache.ip_address == 0 && state->wifi->m_cache.wifi_channel == 0)
            {
                fast_connect_normal(state->WiFiSSID, state->WiFiPassword);
            }
            else
            {
                fast_connect_fast(state->WiFiSSID, state->WiFiPassword, state->wifi->m_cache);
            }
        }

        bool connected(state_t* state)
        {
            if (WiFi.status() == WL_CONNECTED)
            {
                if (state->wifi->m_status != nstatus::Connected)
                {
                    state->wifi->m_status = nstatus::Connected;

                    WiFi.macAddress(state->wifi->m_mac.m_address);

                    nwifi::cache_t& cache = state->wifi->m_cache;
                    cache.ip_address      = WiFi.localIP();
                    cache.ip_gateway      = WiFi.gatewayIP();
                    cache.ip_mask         = WiFi.subnetMask();
                    cache.ip_dns1         = WiFi.dnsIP(0);
                    cache.ip_dns2         = WiFi.dnsIP(1);
                    memcpy(cache.wifi_bssid, WiFi.BSSID(), 6);
                    cache.wifi_channel = WiFi.channel();
                    cache.m_crc        = 0;
                    cache.m_crc        = neeprom::crc32((const byte*)&cache, sizeof(nwifi::cache_t));
                    neeprom::save((const byte*)&cache, sizeof(nwifi::cache_t));
                }
                return true;
            }
            state->wifi->m_status = nstatus::Disconnected;
            return false;
        }

        void disconnect(state_t* state)
        {
            WiFi.disconnect();
            state->wifi->m_status = nstatus::Disconnected;
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

        BSID_t            CurrentBSSID              = {0, 0, 0, 0, 0, 0, 0, 0};
        IPAddress_t       CurrentDNS                = {0, 0, 0, 0};
        IPAddress_t       CurrentGateway            = {127, 0, 0, 255};
        IPAddress_t       CurrentLocalIP            = {127, 0, 0, 1};
        s32               CurrentNetworks           = 0;
        s32               CurrentRSSI               = -1;
        const char*       CurrentSSID               = "";
        nstatus::status_t CurrentStatus             = nstatus::Idle;
        int               SocketPort[MaxSocketNum]  = {0};
        int               SocketState[MaxSocketNum] = {0};

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
