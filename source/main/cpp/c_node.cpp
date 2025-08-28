#ifdef TARGET_ESP32

#    include "Arduino.h"
#    include "WiFi.h"
#    include "rdno_wifi/c_wifi.h"

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

            for (s32 r = 0; r < 3; r++)
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
            }

            nserial::Println("Failed to connect to WiFi.");
            return false;
        }

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

        void NodeUpdate(nvstore::config_t* config, s16 (*funcNameToIndex)(const char* str, s32 len))
        {
            while (true)
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

                            msgLength = 0;  // reset message length for next message

                            // Check if we have valid WiFi configuration
                            const char* ssid          = nvstore::GetString(config, nvstore::PARAM_WIFI_SSID);
                            const char* pass          = nvstore::GetString(config, nvstore::PARAM_WIFI_PASSWORD);
                            const char* remote_server = nvstore::GetString(config, nvstore::PARAM_REMOTE_SERVER_IP);
                            s32         remote_port   = nvstore::GetInt(config, nvstore::PARAM_REMOTE_SERVER_PORT);

                            bool valid_wifi_config   = strlen(ssid) > 0 && strlen(pass) > 0 && IsValidSSID(ssid) && IsValidPassword(pass);
                            bool valid_server_config = strlen(remote_server) > 0 && remote_port > 0 && IsValidIPAddress(remote_server);

                            if (valid_wifi_config && valid_server_config)
                            {
                                break;  // exit the loop if we have valid configuration
                            }
                        }
                    }
                }

                gTcpServer.stop();            // Stop the TCP server
                WiFi.softAPdisconnect(true);  // Stop the access point

                nserial::Println("Configuration access point stopped.");

                if (NodeConnectToWiFi(config))
                    break;
            }
        }

        void NodeSetup(nvstore::config_t* config)
        {
            if (config == nullptr)
                return false;

            NodeUpdate(config);
        }

        bool NodeLoop(nvstore::config_t* config)
        {
            if (nwifi::Status() != nstatus::Connected)
            {
                NodeUpdate(config);
            }

            if (nclient::Connected() != nstatus::Connected)
            {
                nclient::Stop();  // Stop any existing client connection

                nserial::Println("Connecting to server.");
                nstatus::status_t clientStatus = nclient::Connected();
                nstatus::status_t wifiStatus   = nwifi::Status();
                while (clientStatus != nstatus::Connected && wifiStatus == nstatus::Connected)
                {
                    nserial::Println("    Connecting...");
                    ntimer::Delay(2000);
                    clientStatus = nclient::Connect(config->m_remote_server, config->m_remote_port, 10000);  // Reconnect to the server (already has timeout=10 seconds)
                    wifiStatus   = nwifi::Status();
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

                ntimer::Delay(3000);  // Wait for 3 seconds
            }

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

    }  // namespace nwifi
}  // namespace ncore

#endif
