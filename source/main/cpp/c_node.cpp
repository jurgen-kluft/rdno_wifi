#ifdef TARGET_ESP32

#    include "Arduino.h"
#    include "WiFi.h"
#    include "rdno_wifi/c_wifi.h"
#    include "rdno_wifi/c_tcp.h"
#    include "rdno_core/c_str.h"
#    include "rdno_core/c_nvstore.h"

namespace ncore
{
    namespace nwifi
    {
        bool node_connect_to_WiFi(nvstore::config_t* config)
        {
            // Disconnect from WiFi
            nwifi::disconnect();

            // Set WiFi to station mode
            nwifi::set_mode_STA();

            // Wait for 2 seconds
            ntimer::Delay(2000);

            // Connect to WiFi
            nwifi::begin_encrypted(config->m_ssid, config->m_password);

            s32       attempts    = 0;
            const s32 maxAttempts = 6 * 5;  // Try for 1 minute
            while (attempts < maxAttempts)
            {
                nstatus::status_t wifiStatus = nwifi::status();
                nserial::Println("Connecting to WiFi.");
                s32 seconds = 0;
                while (seconds < 10 && wifiStatus != nstatus::Connected)
                {
                    nserial::Println("    Connecting...");
                    ntimer::Delay(2000);
                    seconds += 2;
                    wifiStatus = nwifi::status();
                }

                if (wifiStatus == nstatus::Connected)
                {
                    nserial::Println("Connected to WiFi.");
                    return true;
                }
                attempts++;
            }

            nserial::Println("Failed to connect to WiFi.");
            return false;
        }

        void node_update(nvstore::config_t* config, s16 (*nameToIndex)(str_t const& str))
        {
            const u64 startTimeInMillis       = millis();
            s32       connectToWiFiSeconds    = 0;
            const s32 maxConnectToWiFiSeconds = 5 * 60;  // Try for 5 minutes to connect to WiFi
            while (connectToWiFiSeconds < maxConnectToWiFiSeconds)
            {
                nwifi::disconnect();
                nwifi::set_mode_AP();

                // Start the access point to receive configuration
                str_t ap_ssid     = str_const("esp32");
                str_t ap_password = str_const("1234");
                nvstore::get_string(config, nvstore::PARAM_ID_AP_SSID, ap_ssid);
                nvstore::get_string(config, nvstore::PARAM_ID_AP_PASSWORD, ap_password);

                nwifi::begin_AP(ap_ssid, ap_password);

                nserial::Println("Access Point started to configure settings.");

                // Start the TCP server to listen for configuration data
                ntcp::start_server(31337);

                char  msgBytes[256];
                str_t msg = str_mutable(msgBytes, 256);

                s32       waitForClientSeconds    = 0;
                const s32 maxWaitForClientSeconds = 60;  // Wait for 1 minute for a client to connect
                while (waitForClientSeconds < maxWaitForClientSeconds)
                {
                    ntcp::client_t client = ntcp::server_handle_client();
                    if (client != nullptr)
                    {
                        ntcp::client_recv_msg(client, msg);
                        if (str_len(msg) > 0)
                        {
                            str_t outKey;
                            str_t outValue;
                            while (nvstore::parse_keyvalue(msg, outKey, outValue))
                            {
                                const s16 index = nameToIndex(outKey);
                                nvstore::parse_value(config, index, outValue, outValueLength);
                            }
                            str_clear(msg);

                            // Check if we have valid WiFi configuration
                            str_t ssid          = str_empty();
                            str_t pass          = str_empty();
                            str_t remote_server = str_empty();
                            s32   remote_port   = 0;

                            nvstore::get_string(config, nvstore::PARAM_ID_WIFI_SSID, ssid);
                            nvstore::get_string(config, nvstore::PARAM_ID_WIFI_PASSWORD, pass);
                            nvstore::get_string(config, nvstore::PARAM_ID_REMOTE_SERVER_IP, remote_server);
                            nvstore::get_int(config, nvstore::PARAM_ID_REMOTE_SERVER_PORT, remote_port);

                            const bool valid_wifi_config   = IsValidSSID(ssid) && IsValidPassword(pass);
                            const bool valid_server_config = IsValidIPAddress(remote_server) && IsValidPort(remote_port);
                            if (valid_wifi_config && valid_server_config)
                            {
                                break;  // exit the loop if we have valid configuration
                            }
                        }
                    }
                    else
                    {
                        waitForClientSeconds++;
                        ntimer::Delay(1000);
                    }
                }

                ntcp::stop_server();
                nwifi::disconnect_AP(true);  // Stop the access point

                nserial::Println("Configuration access point stopped.");

                if (node_connect_to_WiFi(config))
                    break;

                // Calculate how long we've been trying to connect to WiFi
                connectToWiFiSeconds = (millis() - startTimeInMillis) / 1000;
            }
        }

        void node_setup(nvstore::config_t* config, s16 (*nameToIndex)(str_t const& str))
        {
            if (config == nullptr)
                return

            node_update(config, nameToIndex);
        }

        void node_loop(nvstore::config_t* config, s16 (*nameToIndex)(str_t const& str))
        {
            if (nwifi::status() != nstatus::Connected || nclient::connected() != nstatus::Connected)
            {
                node_update(config, nameToIndex);
            }

            if (nclient::connected() != nstatus::Connected)
            {
                nclient::stop();  // Stop any existing client connection

                nserial::Println("Connecting to server.");
                nstatus::status_t clientStatus = nclient::connected();
                nstatus::status_t wifiStatus   = nwifi::status();
                s32               attempts     = 0;
                const s32         maxAttempts  = 6 * 5;  // Try for 5 minutes
                while (clientStatus != nstatus::Connected && wifiStatus == nstatus::Connected && attempts < maxAttempts)
                {
                    nserial::Println("    Connecting...");
                    ntimer::Delay(2000);
                    clientStatus = nclient::connect(config->m_remote_server, config->m_remote_port, 8000);  // connect to server
                    wifiStatus   = nwifi::status();
                    attempts++;
                }

                if (wifiStatus == nstatus::Connected && clientStatus == nstatus::Connected)
                {
                    nserial::Println("Connected to server.");

                    IPAddress_t localIP = nclient::local_IP();
                    nserial::Print("IP: ");
                    nserial::PrintIp(localIP);
                    nserial::Println("");

                    MACAddress_t mac = nwifi::mac_address();
                    nserial::Print("MAC: ");
                    nserial::PrintMac(mac);
                    nserial::Println("");
                }
                else
                {
                    nserial::Println("Failed to connect to server.");
                    nclient::stop();  // Stop any existing client connection
                }
            }
        }

    }  // namespace nwifi
}  // namespace ncore

#else

#    include "rdno_wifi/c_wifi.h"

namespace ncore
{
    namespace nwifi
    {
        void node_update(nvstore::config_t* config, s16 (*nameToIndex)(str_t const& str))
        {
            // No update needed for non-ESP32 platforms
        }

        void node_setup(nvstore::config_t* config, s16 (*nameToIndex)(str_t const& str))
        {
            // No setup needed for non-ESP32 platforms
        }
        
        void node_loop(nvstore::config_t* config, s16 (*nameToIndex)(str_t const& str))
        {
            // No loop needed for non-ESP32 platforms
        }

    }  // namespace nwifi
}  // namespace ncore

#endif
