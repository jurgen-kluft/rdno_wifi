#ifdef TARGET_ESP32

#    include "Arduino.h"
#    include "WiFi.h"
#    include "rdno_wifi/c_wifi.h"
#    include "rdno_core/c_str.h"
#    include "rdno_core/c_nvstore.h"

namespace ncore
{
    namespace nwifi
    {
        bool NodeConnectToWiFi(nvstore::config_t* config)
        {
            // Disconnect from WiFi
            nwifi::Disconnect();

            // Set WiFi to station mode
            nwifi::SetModeStation();

            // Wait for 2 seconds
            ntimer::Delay(2000);

            // Connect to WiFi
            nwifi::BeginEncrypted(config->m_ssid, config->m_password);

            s32       attempts    = 0;
            const s32 maxAttempts = 6 * 5; // Try for 1 minute
            while (attempts < maxAttempts)
            {
                nstatus::status_t wifiStatus = nwifi::Status();
                nserial::Println("Connecting to WiFi.");
                s32 seconds = 0;
                while (seconds < 10 && wifiStatus != nstatus::Connected)
                {
                    nserial::Println("    Connecting...");
                    ntimer::Delay(2000);
                    seconds += 2;
                    wifiStatus = nwifi::Status();
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

        // TODO move into rdno_apps
        s16 KeyToIndex(str_t const& str)
        {
            if (str_len(str) == 4 && str_cmp_n(str, "ssid", 4, false) == 0)
                return 0;
            if (str_len(str) == 8 && str_cmp(str, "password", 8, false) == 0)
                return 1;
            if (str_len(str) == 9 && str_cmp(str, "ap_ssid", 7, false) == 0)
                return 2;
            if (str_len(str) == 11 && str_cmp(str, "ap_password", 11, false) == 0)
                return 3;
            if (str_len(str) == 13 && str_cmp(str, "remote_server", 13, false) == 0)
                return 4;
            if (str_len(str) == 11 && str_cmp(str, "remote_port", 11, false) == 0)
                return 5;

            s32 value = 0;
            if (from_str(str, &value, 10) && value >= 0 && value < 256)
            {
                return static_cast<s16>(value);
            }
            return -1;
        }

        void NodeUpdate(nvstore::config_t* config, s16 (*funcNameToIndex)(str_t const& str))
        {
            s32       connectToWiFiSeconds    = 0;
            const s32 maxConnectToWiFiSeconds = 5 * 60; // Try for 5 minutes to connect to WiFi
            while (connectToWiFiSeconds < maxConnectToWiFiSeconds)
            {
                nwifi::Disconnect();
                nwifi::SetModeAP();

                // Start the access point to receive configuration
                str_t ap_ssid     = str_const("esp32");
                str_t ap_password = str_const("1234");
                nvstore::GetString(config, nvstore::PARAM_ID_AP_SSID, ap_ssid);
                nvstore::GetString(config, nvstore::PARAM_ID_AP_PASSWORD, ap_password);

                WiFi.softAP(ap_ssid, ap_password);

                nserial::Println("Access Point started to configure settings.");

                // Start the TCP server to listen for configuration data
                gTcpServer.begin();

                char  msgBytes[256];
                str_t msg = str_mutable(msgBytes, 256);

                s32       waitForClientSeconds    = 0;
                const s32 maxWaitForClientSeconds = 60; // Wait for 1 minute for a client to connect
                while (waitForClientSeconds < maxWaitForClientSeconds)
                {
                    // Wait for a client to connect
                    WiFiClient client = gTcpServer.available();
                    if (client)
                    {
                        // TODO client connected, now read message

                        if (msgLength > 0)
                        {
                            str_t outKey;
                            str_t outValue;
                            while (nvstore::ParseKeyValue(msg, outKey, outValue))
                            {
                                const s16 index = funcNameToIndex(outKey);
                                nvstore::ParseValue(config, index, outValue, outValueLength);
                            }
                            str_clear(msg);

                            // Check if we have valid WiFi configuration
                            str_t ssid          = str_empty();
                            str_t pass          = str_empty();
                            str_t remote_server = str_empty();
                            s32   remote_port   = 0;

                            nvstore::GetString(config, nvstore::PARAM_ID_WIFI_SSID, ssid);
                            nvstore::GetString(config, nvstore::PARAM_ID_WIFI_PASSWORD, pass);
                            nvstore::GetString(config, nvstore::PARAM_ID_REMOTE_SERVER_IP, remote_server);
                            nvstore::GetInt(config, nvstore::PARAM_ID_REMOTE_SERVER_PORT, remote_port);

                            const bool valid_wifi_config   = IsValidSSID(ssid) && IsValidPassword(pass);
                            const bool valid_server_config = IsValidIPAddress(remote_server) && IsValidPort(remote_port);
                            if (valid_wifi_config && valid_server_config)
                            {
                                break; // exit the loop if we have valid configuration
                            }
                        }
                    }
                    else
                    {
                        waitForClientSeconds++;
                        ntimer::Delay(1000);
                    }
                }

                gTcpServer.stop();           // Stop the TCP server
                WiFi.softAPdisconnect(true); // Stop the access point

                nserial::Println("Configuration access point stopped.");

                if (NodeConnectToWiFi(config))
                    break;
            }
        }

        void NodeSetup(nvstore::config_t* config)
        {
            if (config == nullptr)
                return

                    NodeUpdate(config);
        }

        void NodeLoop(nvstore::config_t* config)
        {
            if (nwifi::Status() != nstatus::Connected || nclient::Connected() != nstatus::Connected)
            {
                NodeUpdate(config);
            }

            if (nclient::Connected() != nstatus::Connected)
            {
                nclient::Stop(); // Stop any existing client connection

                nserial::Println("Connecting to server.");
                nstatus::status_t clientStatus = nclient::Connected();
                nstatus::status_t wifiStatus   = nwifi::Status();
                s32               attempts     = 0;
                const s32         maxAttempts  = 6 * 5; // Try for 5 minutes
                while (clientStatus != nstatus::Connected && wifiStatus == nstatus::Connected && attempts < maxAttempts)
                {
                    nserial::Println("    Connecting...");
                    ntimer::Delay(2000);
                    clientStatus = nclient::Connect(config->m_remote_server, config->m_remote_port, 8000); // connect to server
                    wifiStatus   = nwifi::Status();
                    attempts++;
                }

                if (wifiStatus == nstatus::Connected && clientStatus == nstatus::Connected)
                {
                    nserial::Println("Connected to server.");

                    IPAddress_t localIP = nclient::LocalIP();
                    nserial::Print("IP: ");
                    nserial::PrintIp(localIP);
                    nserial::Println("");

                    MACAddress_t mac = nwifi::MacAddress();
                    nserial::Print("MAC: ");
                    nserial::PrintMac(mac);
                    nserial::Println("");
                }
                else
                {
                    nserial::Println("Failed to connect to server.");
                    nclient::Stop(); // Stop any existing client connection
                }
            }
        }

    } // namespace nwifi
} // namespace ncore

#else

#    include "rdno_wifi/c_wifi.h"

namespace ncore
{
    namespace nwifi
    {

    } // namespace nwifi
} // namespace ncore

#endif
