#ifdef TARGET_ARDUINO

#    include "Arduino.h"
#    include "WiFi.h"

#    ifdef TARGET_ESP8266
#        include "ESP8266WiFi.h"
#    endif

#    include "rdno_wifi/c_wifi.h"
#    include "rdno_core/c_eeprom.h"
#    include "rdno_core/c_network.h"
#    include "rdno_core/c_serial.h"
#    include "rdno_core/c_str.h"
#    include "ccore/c_memory.h"

namespace ncore
{
    namespace nwifi
    {
        state_wifi_t gWiFiState;

        void cache_t::reset() { g_memclr(this, sizeof(cache_t)); }

        void init_state(state_t* state, bool load_cache)
        {
            state->wifi           = &gWiFiState;
            state->wifi->m_status = nstatus::Disconnected;

            if (load_cache)
            {
                ncore::nserial::println("loading WiFi cache from EEPROM");
                ncore::neeprom::load((byte*)&state->wifi->m_cache, sizeof(ncore::nwifi::cache_t));
                {
                    const u32 crc              = state->wifi->m_cache.m_crc;
                    state->wifi->m_cache.m_crc = 0;
                    if (crc != neeprom::crc32((const byte*)&state->wifi->m_cache, sizeof(nwifi::cache_t)))
                    {
                        ncore::nserial::println(" WiFi cache in EEPROM is corrupted (CRC mismatch)");
                        state->wifi->m_cache.reset();
                    }
                    else
                    {
                        ncore::nserial::println("WiFi cache loaded from EEPROM");
                        state->wifi->m_cache.m_crc = crc;
                    }
                }
            }
            else
            {
                state->wifi->m_cache.reset();
            }
        }

        void disconnect() { WiFi.disconnect(); }
        void disconnect_AP(bool wifioff) { WiFi.softAPdisconnect(wifioff); }

        IPAddress_t local_IP()
        {
            IPAddress   ip = WiFi.localIP();
            IPAddress_t ipAddr;
            ipAddr.from(ip[0], ip[1], ip[2], ip[3]);
            return ipAddr;
        }

        MACAddress_t mac_address()
        {
            MACAddress_t macAddress;
            uint8_t*     mac = WiFi.macAddress(&macAddress.m_address[0]);
            return macAddress;
        }

        s32 get_RSSI(state_t* state) { return WiFi.RSSI(); }

        void set_DNS(const IPAddress_t& dns)
        {
            IPAddress ip(dns.m_address);
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
            WiFi.config(cache.ip_address, cache.ip_gateway, cache.ip_mask);
            WiFi.begin(ssid, auth, cache.wifi_channel, cache.wifi_bssid, true);
        }

        // do a normal wifi connection, once connected cache connection info, return true if ok
        void fast_connect_normal(const char* ssid, const char* auth) { WiFi.begin(ssid, auth); }

        // Connect to wifi as specified, returns true if ok
        void connect(state_t* state)
        {
            WiFi.setAutoReconnect(false);  // prevent early autoconnect
            WiFi.persistent(true);
            WiFi.mode(WIFI_STA);

            if (state->wifi->m_cache.ip_address == 0)
            {
                nserial::printf("Connect (normal) to WiFi with SSID %s ...\n", va_t(state->WiFiSSID));
                fast_connect_normal(state->WiFiSSID, state->WiFiPassword);
            }
            else
            {
                nserial::printf("Connect (fast) to WiFi with SSID %s ...\n", va_t(state->WiFiSSID));
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
                    WiFi.BSSID(cache.wifi_bssid);
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

        void print_connection_info(state_t* state)
        {
            ncore::nserial::println("WiFi Connection Info:");

#    ifdef TARGET_ESP8266
            // Print PhyMode
            if (WiFi.getPhyMode() == WIFI_PHY_MODE_11B)
            {
                ncore::nserial::println(" PhyMode: 802.11b");
            }
            else if (WiFi.getPhyMode() == WIFI_PHY_MODE_11G)
            {
                ncore::nserial::println(" PhyMode: 802.11g");
            }
            else if (WiFi.getPhyMode() == WIFI_PHY_MODE_11N)
            {
                ncore::nserial::println(" PhyMode: 802.11n");
            }
            else
            {
                ncore::nserial::println(" PhyMode: Unknown");
            }
#    endif

            ncore::nserial::print(" SSID: ");
            ncore::nserial::println(state->WiFiSSID);
            ncore::nserial::print(" MAC Address: ");
            MACAddress_t mac = state->wifi->m_mac;
            ncore::nserial::print(mac);
            ncore::nserial::println("");
            IPAddress ip = WiFi.localIP();
            ncore::nserial::printf("  IP Address: %d.%d.%d.%d\n", va_t(ip[0]), va_t(ip[1]), va_t(ip[2]), va_t(ip[3]));
            ncore::nserial::printf(" RSSI: %d dBm\n", va_t(WiFi.RSSI()));
        }

    }  // namespace nwifi
}  // namespace ncore

#else

#    include "rdno_wifi/c_wifi.h"

namespace ncore
{
    namespace nwifi
    {
        void init_state(state_t* state) {}
        void connect(state_t* state) {}
        bool connected(state_t* state) { return false; }
        void disconnect(state_t* state) {}

        void print_connection_info(state_t* state) {}

    }  // namespace nwifi
}  // namespace ncore

#endif
