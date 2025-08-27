#ifdef TARGET_ESP32

#    include "Arduino.h"
#    include "WiFi.h"
#    include "rdno_wifi/c_wifi.h"

namespace ncore
{
    namespace nwifi
    {
        bool NodeUpdateConfig(nvstore::config_t* config)
        {
            // If config SSID or password is empty, we will wait until we receive a configuration
            // that does have valid SSID and password
            bool valid_wifi_config = false;
            bool valid_server_config = false;
            while (!valid_wifi_config)
            {
                nwifi::UpdateConfig(config);

                valid_wifi_config = strlen(config->m_ssid) > 0 && strlen(config->m_password) > 0;
                valid_server_config = strlen(config->m_remote_server) > 0 && config->m_remote_port > 0;
            }
        }

        bool NodeConnectToWiFi(nvstore::config_t* config)
        {
            // Disconnect from WiFi
            nwifi::Disconnect();

            ntimer::Delay(5000);

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

        bool NodeSetup(nvstore::config_t* config)
        {
            if (config == nullptr)
                return false;

            // Set WiFi to both station and access point mode
            nwifi::SetModeAPSTA();

            while (!NodeUpdateConfig(config))
            {
                // Update config until we have a valid WiFi and Remote Server configuration
            }

            return NodeConnectToWiFi(config);
        }

        bool NodeLoop(nvstore::config_t* config)
        {
            if (nwifi::Status() != nstatus::Connected)
            {
                while (!NodeConnectToWiFi(config))
                {
                    while (!NodeUpdateConfig(config))
                    {
                        // Update config until we have a valid WiFi and Remote Server configuration
                    }
                }
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
                    ntimer::Delay(5000);
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
