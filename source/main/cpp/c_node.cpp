#ifdef TARGET_ESP32

#    include "Arduino.h"
#    include "WiFi.h"
#    include "rdno_core/c_nvstore.h"
#    include "rdno_core/c_serial.h"
#    include "rdno_core/c_str.h"
#    include "rdno_core/c_timer.h"

#    include "rdno_wifi/c_remote.h"
#    include "rdno_wifi/c_tcp.h"
#    include "rdno_wifi/c_wifi.h"

namespace ncore
{
    namespace nwifi
    {
        enum EWiFiConnectState
        {
            WIFI_CONNECT_BEGIN,
            WIFI_CONNECT_CONNECTING,
            WIFI_CONNECT_CONNECTED,
            WIFI_CONNECT_ERROR,
        };

        EWiFiConnectState sWiFiConnectState = WIFI_CONNECT_BEGIN;
        u64               sWiFiConnectStartTimeInMillis;

        s32 node_connect_to_WiFi(nvstore::config_t* config)
        {
            switch (sWiFiConnectState)
            {
                case WIFI_CONNECT_BEGIN:
                {
                    nwifi::disconnect();    // Disconnect from WiFi
                    nwifi::set_mode_STA();  // Set WiFi to station mode

                    str_t ssid;
                    if (!nvstore::get_string(config, nvstore::PARAM_ID_SSID, ssid))
                        return -1;
                    str_t pass;
                    if (!nvstore::get_string(config, nvstore::PARAM_ID_PASSWORD, pass))
                        return -1;

                    nserial::println("Connecting to WiFi.");
                    nwifi::begin_encrypted(ssid.m_const, pass.m_const);  // Connect to WiFi
                    sWiFiConnectStartTimeInMillis = millis();
                    sWiFiConnectState             = WIFI_CONNECT_CONNECTING;
                }
                break;
                case WIFI_CONNECT_CONNECTING:
                    if (nwifi::status() == nstatus::Connected)
                    {
                        nserial::println("  -> Connected");
                        sWiFiConnectState = WIFI_CONNECT_CONNECTED;
                    }
                    else
                    {
                        const u64 currentTimeInMillis = millis();
                        if (currentTimeInMillis - sWiFiConnectStartTimeInMillis > 5 * 60 * 1000)  // 5 minutes timeout
                        {
                            nserial::println("  -> Failed to connect to WiFi (timeout).");
                            sWiFiConnectState = WIFI_CONNECT_ERROR;
                            return -1;
                        }
                    }
                    break;
            }

            return sWiFiConnectState == WIFI_CONNECT_CONNECTED ? 1 : 0;
        }

        enum ERemoteConnectState
        {
            REMOTE_CONNECT_BEGIN,
            REMOTE_CONNECT_CONNECTING,
            REMOTE_CONNECT_CONNECTED,
            REMOTE_CONNECT_ERROR,
        };

        ERemoteConnectState sRemoteConnectState = REMOTE_CONNECT_BEGIN;
        u64                 sRemoteConnectStartTimeInMillis;

        s32 node_connect_to_remote(nvstore::config_t* config)
        {
            switch (sRemoteConnectState)
            {
                case REMOTE_CONNECT_BEGIN:
                {
                    nremote::stop();  // Stop any existing client connection

                    nserial::println("Connecting to remote.");
                    if (nwifi::status() != nstatus::Connected)
                    {
                        nserial::println("  -> Unable to connect to remote, WiFi not connected.");
                        sRemoteConnectState = REMOTE_CONNECT_ERROR;
                        return -1;
                    }
                }
                break;

                case REMOTE_CONNECT_CONNECTING:
                {
                    str_t remote_server;
                    if (!nvstore::get_string(config, nvstore::PARAM_ID_REMOTE_SERVER, remote_server))
                        return -1;
                    const s32 remote_port = nvstore::get_int(config, nvstore::PARAM_ID_REMOTE_PORT, 0);

                    nstatus::status_t clientStatus = nremote::connect(remote_server.m_const, remote_port, 5000);
                    if (clientStatus == nstatus::Connected)
                    {
                        nserial::println("  -> Connected to remote.");

                        IPAddress_t localIP = nremote::local_IP();
                        nserial::print("     IP: ");
                        nserial::print(localIP);
                        nserial::println("");

                        MACAddress_t mac = nwifi::mac_address();
                        nserial::print("     MAC: ");
                        nserial::print(mac);
                        nserial::println("");

                        sRemoteConnectState = REMOTE_CONNECT_CONNECTED;
                        return true;
                    }

                    const u64 currentTimeInMillis = millis();
                    if (currentTimeInMillis - sRemoteConnectStartTimeInMillis > 5 * 60 * 1000)  // 5 minutes timeout
                    {
                        nserial::println("  -> Failed to connect to remote (timeout).");
                        sRemoteConnectState = REMOTE_CONNECT_ERROR;
                        return -1;
                    }
                }
                break;
            }
            return sRemoteConnectState == REMOTE_CONNECT_CONNECTED ? 1 : 0;
        }

        enum ENodeState
        {
            NODE_CONFIG_BEGIN,
            NODE_CONFIG_RECEIVE,
            NODE_WIFI_CONNECTING,
            NODE_REMOTE_CONNECTING,
            NODE_CONNECTED,
        };

        ENodeState sState = NODE_CONFIG_BEGIN;
        char       msgBytes[256];
        str_t      msg;

        bool node_update(nvstore::config_t* config, s16 (*nameToIndex)(str_t const& str))
        {
            switch (sState)
            {
                case NODE_CONFIG_BEGIN:
                {
                    nwifi::disconnect();
                    nwifi::set_mode_AP();

                    // Start the access point to receive configuration
                    str_t ap_ssid = str_const("esp32");
                    nvstore::get_string(config, nvstore::PARAM_ID_AP_SSID, ap_ssid);
                    str_t ap_password = str_const("1234");
                    nvstore::get_string(config, nvstore::PARAM_ID_AP_PASSWORD, ap_password);

                    nwifi::begin_AP(ap_ssid.m_const, ap_password.m_const);

                    nserial::println("Access Point started to receive configuration");

                    // Start the TCP server to receive configuration data
                    ntcp::server_start(31337);
                    msg = str_mutable(msgBytes, 256);

                    sState = NODE_CONFIG_RECEIVE;
                }
                break;

                case NODE_CONFIG_RECEIVE:
                {
                    // TODO should we timeout here if no clients connect within a certain time?

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
                                nvstore::parse_value(config, index, outValue);
                            }
                            str_clear(msg);

                            // Check if we have valid WiFi configuration
                            str_t ssid          = str_empty();
                            str_t pass          = str_empty();
                            str_t remote_server = str_empty();
                            s32   remote_port   = 0;

                            nvstore::get_string(config, nvstore::PARAM_ID_SSID, ssid);
                            nvstore::get_string(config, nvstore::PARAM_ID_PASSWORD, pass);
                            nvstore::get_string(config, nvstore::PARAM_ID_REMOTE_SERVER, remote_server);
                            nvstore::get_int(config, nvstore::PARAM_ID_REMOTE_PORT, remote_port);

                            const bool valid_wifi_config   = is_valid_SSID(ssid) && is_valid_password(pass);
                            const bool valid_server_config = is_valid_IPAddress(remote_server) && is_valid_port(remote_port);
                            if (valid_wifi_config && valid_server_config)
                            {
                                ntcp::server_stop();         // Stop the TCP server
                                nwifi::disconnect_AP(true);  // Stop the access point

                                nserial::println("Access point, received configuration.");

                                sState = NODE_WIFI_CONNECTING;
                            }
                        }
                    }
                }
                break;

                case NODE_WIFI_CONNECTING:
                {
                    const s32 result = node_connect_to_WiFi(config);
                    if (result == 1)
                    {
                        sState = NODE_REMOTE_CONNECTING;
                    }
                    else if (result == -1)
                    {
                        // Failed to connect to WiFi, go back to configuration setup since we might have bad configuration
                        sState = NODE_CONFIG_BEGIN;
                    }
                }
                break;

                case NODE_REMOTE_CONNECTING:
                {
                    const s32 result = node_connect_to_remote(config);
                    if (result == 1)
                    {
                        sState = NODE_CONNECTED;
                    }
                    else if (result == -1)
                    {
                        sState = NODE_CONFIG_BEGIN;
                    }
                }
                break;

                case NODE_CONNECTED:
                    if (nwifi::status() != nstatus::Connected)
                    {
                        nserial::println("Lost connection to WiFi, reconnecting...");
                        sWiFiConnectState   = WIFI_CONNECT_BEGIN;
                        sRemoteConnectState = REMOTE_CONNECT_BEGIN;
                        sState              = NODE_WIFI_CONNECTING;
                    }
                    else if (nremote::connected() != nstatus::Connected)
                    {
                        nserial::println("Lost connection to remote, reconnecting...");
                        sRemoteConnectState = REMOTE_CONNECT_BEGIN;
                        sState              = NODE_REMOTE_CONNECTING;
                    }
                    break;
            }

            return sState == NODE_CONNECTED;
        }

        void node_setup(nvstore::config_t* config, s16 (*nameToIndex)(str_t const& str))
        {
            sWiFiConnectState   = WIFI_CONNECT_BEGIN;
            sRemoteConnectState = REMOTE_CONNECT_BEGIN;

            // Start by assuming we have a valid configuration, this means we will try to connect to WiFi and remote server
            // If unable to connect to WiFi and/or Remote, we will move into the configuration setup state
            sState = NODE_WIFI_CONNECTING;

            if (config != nullptr)
                node_update(config, nameToIndex);
        }

        bool node_loop(nvstore::config_t* config, s16 (*nameToIndex)(str_t const& str)) { return config != nullptr && node_update(config, nameToIndex); }

    }  // namespace nwifi
}  // namespace ncore

#else

#    include "rdno_wifi/c_wifi.h"

namespace ncore
{
    namespace nwifi
    {
        void node_setup(nvstore::config_t* config, s16 (*nameToIndex)(str_t const& str))
        {
            // No setup needed for non-ESP32 platforms
        }

        bool node_loop(nvstore::config_t* config, s16 (*nameToIndex)(str_t const& str))
        {
            // No loop needed for non-ESP32 platforms
            return true;
        }

    }  // namespace nwifi
}  // namespace ncore

#endif
