#ifdef TARGET_ESP32

#    include "Arduino.h"
#    include "WiFi.h"

#    include "rdno_core/c_app.h"
#    include "rdno_core/c_config.h"
#    include "rdno_core/c_packet.h"
#    include "rdno_core/c_network.h"
#    include "rdno_core/c_nvstore.h"
#    include "rdno_core/c_serial.h"
#    include "rdno_core/c_str.h"
#    include "rdno_core/c_task.h"
#    include "rdno_core/c_timer.h"

#    include "rdno_wifi/c_remote.h"
#    include "rdno_wifi/c_tcp.h"
#    include "rdno_wifi/c_wifi.h"
#    include "rdno_wifi/c_node.h"

namespace ncore
{
#    if 0
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

        s32 node_connect_to_WiFi(nconfig::config_t* config)
        {
            switch (sWiFiConnectState)
            {
                case WIFI_CONNECT_BEGIN:
                {
                    nwifi::set_mode_STA();  // Set WiFi to station mode

                    str_t ssid;
                    if (!nconfig::get_string(config, nconfig::PARAM_ID_WIFI_SSID, ssid))
                        return -1;
                    str_t pass;
                    if (!nconfig::get_string(config, nconfig::PARAM_ID_WIFI_PASSWORD, pass))
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
        u64                 sNodeTimeSync = 0;

        u64 node_timesync()
        {
            const u64 millis = ntimer::millis();
            return (millis - sNodeTimeSync);
        }

        s32 node_connect_to_remote(nconfig::config_t* config)
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

                    sRemoteConnectStartTimeInMillis = millis();
                    sRemoteConnectState             = REMOTE_CONNECT_CONNECTING;
                }
                break;

                case REMOTE_CONNECT_CONNECTING:
                {
                    str_t remote_server;
                    if (!nconfig::get_string(config, nconfig::PARAM_ID_REMOTE_SERVER, remote_server))
                    {
                        nserial::println("  -> Remote server parameter not available.");
                        return -1;
                    }
                    s32 remote_port = 0;
                    if (!nconfig::get_int(config, nconfig::PARAM_ID_REMOTE_PORT, remote_port))
                    {
                        nserial::println("  -> Remote server port parameter not available.");
                        return -1;
                    }

                    nstatus::status_t clientStatus = nremote::connect(remote_server.m_const, remote_port, 8000);
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

                        npacket::packet_t macPacket;
                        macPacket.begin(0, true);
                        const u64 macValue = mac.m_address[0] | (u64(mac.m_address[1]) << 8) | (u64(mac.m_address[2]) << 16) | (u64(mac.m_address[3]) << 24) | (u64(mac.m_address[4]) << 32) | (u64(mac.m_address[5]) << 40);
                        macPacket.write_value(npacket::ntype::MacAddress, macValue);
                        macPacket.finalize();

                        nremote::write(macPacket.Data, macPacket.Size);
                        sNodeTimeSync = ntimer::millis();

                        sRemoteConnectState = REMOTE_CONNECT_CONNECTED;
                        return true;
                    }

                    const u64 currentTimeInMillis = millis();
                    if (currentTimeInMillis - sRemoteConnectStartTimeInMillis > 60 * 1000)
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

        bool node_update(nconfig::config_t* config, s16 (*nameToIndex)(str_t const& str))
        {
            switch (sState)
            {
                case NODE_CONFIG_BEGIN:
                {
                    nwifi::disconnect();
                    nwifi::set_mode_AP();

                    // Start the access point to receive configuration
                    str_t ap_ssid = str_const("esp32");
                    nconfig::get_string(config, nconfig::PARAM_ID_AP_SSID, ap_ssid);
                    str_t ap_password = str_const("1234");
                    nconfig::get_string(config, nconfig::PARAM_ID_AP_PASSWORD, ap_password);

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
                            nserial::println("Access Point, received message from connected client");
                            str_t outKey;
                            str_t outValue;
                            while (nconfig::parse_keyvalue(msg, outKey, outValue))
                            {
                                const s16 index = nameToIndex(outKey);
                                nserial::print("Parse ");
                                str_print(outKey);
                                nserial::print(" = ");
                                str_print(outValue);
                                nserial::print(", index = ");
                                nserial::print((s32)index);
                                nserial::println("");

                                nconfig::parse_value(config, index, outValue);
                            }
                            str_clear(msg);

                            // Check if we have valid WiFi configuration
                            str_t ssid          = str_empty();
                            str_t pass          = str_empty();
                            str_t remote_server = str_empty();
                            s32   remote_port   = 0;

                            nconfig::get_string(config, nconfig::PARAM_ID_WIFI_SSID, ssid);
                            nconfig::get_string(config, nconfig::PARAM_ID_WIFI_PASSWORD, pass);
                            nconfig::get_string(config, nconfig::PARAM_ID_REMOTE_SERVER, remote_server);
                            nconfig::get_int(config, nconfig::PARAM_ID_REMOTE_PORT, remote_port);

                            nserial::println("Access point, new configuration:");
                            nserial::print("    SSID: ");
                            nserial::println(ssid.m_const);
                            nserial::print("    PASSWORD: ");
                            nserial::println(pass.m_const);
                            nserial::print("    REMOTE_SERVER: ");
                            nserial::println(remote_server.m_const);
                            nserial::print("    REMOTE_PORT: ");
                            nserial::print(remote_port);
                            nserial::println("");

                            const bool valid_wifi_config   = is_valid_SSID(ssid) && is_valid_password(pass);
                            const bool valid_server_config = is_valid_IPAddress(remote_server) && is_valid_port(remote_port);
                            if (valid_wifi_config && valid_server_config)
                            {
                                ntcp::server_stop();          // Stop the TCP server
                                nwifi::disconnect_AP(false);  // Stop the access point

                                nserial::println("Access point, new configuration approved.");
                                nconfig::save(config);  // Save the configuration to non-volatile storage

                                sState              = NODE_WIFI_CONNECTING;
                                sWiFiConnectState   = WIFI_CONNECT_BEGIN;
                                sRemoteConnectState = REMOTE_CONNECT_BEGIN;
                            }
                            else
                            {
                                nserial::println("Access point, invalid configuration received, waiting for new configuration.");
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
                        sRemoteConnectState = REMOTE_CONNECT_BEGIN;
                        sState              = NODE_REMOTE_CONNECTING;
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

        void node_setup(nconfig::config_t* config, s16 (*nameToIndex)(str_t const& str))
        {
            sWiFiConnectState   = WIFI_CONNECT_BEGIN;
            sRemoteConnectState = REMOTE_CONNECT_BEGIN;

            // Start by assuming we have a valid configuration, this means we will try to connect to WiFi and remote server
            // If unable to connect to WiFi and/or Remote, we will move into the configuration setup state
            sState = NODE_WIFI_CONNECTING;

            if (config != nullptr)
                node_update(config, nameToIndex);
        }

        bool node_loop(nconfig::config_t* config, s16 (*nameToIndex)(str_t const& str)) { return config != nullptr && node_update(config, nameToIndex); }
    }  // namespace nwifi
#    endif

    namespace nnode
    {
        // ----------------------------------------------------------------
        // ----------------------------------------------------------------
        // ntask based version
        // ----------------------------------------------------------------
        // ----------------------------------------------------------------

        ntask::result_t func_configure_start(ntask::state_t* state)
        {
            nwifi::set_mode_AP();

            // Start the access point to receive configuration
            str_t ap_ssid = str_const("esp32");
            nconfig::get_string(state->config, nconfig::PARAM_ID_AP_SSID, ap_ssid);
            str_t ap_password = str_const("1234");
            nconfig::get_string(state->config, nconfig::PARAM_ID_AP_PASSWORD, ap_password);

            nwifi::begin_AP(ap_ssid.m_const, ap_password.m_const);
            nserial::printf("AP started on %s to receive configuration\n", va_t(ap_ssid.m_const));

            // Start the TCP server to receive configuration data
            ntcp::server_start(31337);
            return ntask::RESULT_DONE;
        }

        ntask::result_t func_configure_loop(ntask::state_t* state)
        {
            ntcp::client_t client = ntcp::server_handle_client();
            if (client != nullptr)
            {
                char  msgBytes[256];
                str_t msg = str_mutable(msgBytes, 256);

                ntcp::client_recv_msg(client, msg);
                if (str_len(msg) > 0)
                {
                    nserial::println("Access Point, received message from connected client");
                    str_t outKey;
                    str_t outValue;
                    while (nconfig::parse_keyvalue(msg, outKey, outValue))
                    {
                        // const s16 index = nameToIndex(outKey);
                        const s16 index = napp::config_key_to_index(outKey);
                        nserial::printf("Parse %s = %s, index = %d\n", va_t(outKey.m_const + outKey.m_str), va_t(outValue.m_const + outValue.m_str), va_t(index));
                        nconfig::parse_value(state->config, index, outValue);
                    }
                    str_clear(msg);

                    // Check if we have valid WiFi configuration
                    str_t ssid          = str_empty();
                    str_t pass          = str_empty();
                    str_t remote_server = str_empty();
                    u16   remote_port   = 0;

                    nconfig::get_string(state->config, nconfig::PARAM_ID_WIFI_SSID, ssid);
                    nconfig::get_string(state->config, nconfig::PARAM_ID_WIFI_PASSWORD, pass);
                    nconfig::get_string(state->config, nconfig::PARAM_ID_REMOTE_SERVER, remote_server);
                    nconfig::get_uint16(state->config, nconfig::PARAM_ID_REMOTE_PORT, remote_port);

                    nserial::printf("Access point, new configuration:\n    SSID: %s\n    PASSWORD: %s\n    REMOTE_SERVER: %s\n    REMOTE_PORT: %d\n", va_t(ssid.m_const + ssid.m_str), va_t(pass.m_const + pass.m_str), va_t(remote_server.m_const + remote_server.m_str), va_t(remote_port));

                    const bool valid_wifi_config   = is_valid_SSID(ssid) && is_valid_password(pass);
                    const bool valid_server_config = is_valid_IPAddress(remote_server) && is_valid_port(remote_port);
                    if (valid_wifi_config && valid_server_config)
                    {
                        ntcp::server_stop();          // Stop the TCP server
                        nwifi::disconnect_AP(false);  // Stop the access point

                        nserial::println("Access point, new configuration approved.");
                        nvstore::save(state->config);  // Save the configuration to non-volatile storage

                        return ntask::RESULT_DONE;
                    }
                    else
                    {
                        nserial::println("Access point, invalid configuration received, waiting for new configuration.");
                    }
                }

                return ntask::RESULT_OK;
            }
        }

        ntask::result_t func_connect_to_WiFi_start(ntask::state_t* state)
        {
            // TODO, can we check the state of the WiFi system, perhaps it is still in STATION mode and connected?

            nwifi::set_mode_STA();  // Set WiFi to station mode

            str_t ssid;
            if (!nconfig::get_string(state->config, nconfig::PARAM_ID_WIFI_SSID, ssid))
                return ntask::RESULT_ERROR;
            str_t pass;
            if (!nconfig::get_string(state->config, nconfig::PARAM_ID_WIFI_PASSWORD, pass))
                return ntask::RESULT_ERROR;

            nserial::printf("Connecting to WiFi with SSID %s ...\n", va_t(ssid.m_const));

            nwifi::begin_encrypted(ssid.m_const, pass.m_const);  // Connect to WiFi
            
            return ntask::RESULT_DONE;
        }

        ntask::result_t func_wifi_is_connected(ntask::state_t* state)
        {
            if (nwifi::status() == nstatus::Connected)
            {
                return ntask::RESULT_DONE;
            }

            return ntask::RESULT_OK;
        }

        ntask::result_t func_wifi_disconnect(ntask::state_t* state)
        {
            nwifi::disconnect();
            return ntask::RESULT_DONE;
        }

        ntask::result_t func_connect_to_remote_start(ntask::state_t* state)
        {
            nremote::stop();  // Stop any existing client connection

            if (nwifi::status() != nstatus::Connected)
            {
                nserial::println("  -> Connecting to remote stopped, WiFi not connected.");
                return ntask::RESULT_ERROR;
            }

            nserial::println("Connecting to remote.");
            return ntask::RESULT_DONE;
        }

        ntask::result_t func_remote_connecting(ntask::state_t* state)
        {
            nconfig::config_t* config = state->config;

            str_t remote_server;
            if (!nconfig::get_string(config, nconfig::PARAM_ID_REMOTE_SERVER, remote_server))
            {
                nserial::println("  -> Remote server parameter not available.");
                return ntask::RESULT_ERROR;
            }
            u16 remote_port = 0;
            if (!nconfig::get_uint16(config, nconfig::PARAM_ID_REMOTE_PORT, remote_port))
            {
                nserial::println("  -> Remote server port parameter not available.");
                return ntask::RESULT_ERROR;
            }

            IPAddress_t remote_server_ip_address;
            from_string(remote_server, remote_server_ip_address);

            nserial::printf("Connecting to %s:%d (IP: %d.%d.%d.%d) ...\n", va_t(remote_server.m_const), va_t(remote_port), va_t(remote_server_ip_address.m_address[0]), va_t(remote_server_ip_address.m_address[1]), va_t(remote_server_ip_address.m_address[2]), va_t(remote_server_ip_address.m_address[3]));

            nstatus::status_t clientStatus = nremote::connect(remote_server_ip_address, remote_port, 8000);
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

                state->time_ms   = ntimer::millis();
                state->time_sync = ntimer::millis();

                return ntask::RESULT_DONE;
            }

            return ntask::RESULT_OK;
        }

        ntask::result_t func_remote_is_connected(ntask::state_t* state)
        {
            if (nremote::connected() == nstatus::Connected)
            {
                return ntask::RESULT_DONE;
            }

            return ntask::RESULT_OK;
        }        

        void connected(ntask::executor_t* scheduler, ntask::program_t main, ntask::state_t* state)
        {
            ntask::program_t node_configure_program  = ntask::program(scheduler, "configuration program");
            ntask::program_t node_connected_program  = ntask::program(scheduler, "connected program");
            ntask::program_t node_connecting_program = ntask::program(scheduler, "connecting program");

            // Start the node program by jumping to the connecting program
            ntask::boot(scheduler, node_connecting_program);

            // The configure program will start an access point and TCP server to receive configuration data.
            // When valid configuration is received it will jump to the connecting program, otherwise it will
            // keep waiting for valid configuration.
            // Once the connecting program is started it will try to connect to WiFi and the Remote, when both
            // are connected it will jump to the connected program.
            // The connected program will check if we are still connected to both WiFi and Remote, and will
            // call the main program. If either WiFi or Remote are disconnected it will jump to the connecting
            // program to reconnect.

            // TODO Do we need a timeout on this program, that no matter what we jump to the connecting program
            //      to try to connect to WiFi and Remote?
            ntask::xbegin(scheduler, node_configure_program);
            {
                ntask::xonce(scheduler, func_configure_start);
                ntask::xif(scheduler, func_configure_loop);
                {
                    ntask::xjump(scheduler, node_connecting_program);
                }
                ntask::xend(scheduler);

                // If we time out while waiting for configuration, we jump to the connecting program
                // Note: Currently set to 5 minutes
                ntask::xif(scheduler, ntask::timeout_t(5 * 60 * 1000));
                {
                    ntask::xjump(scheduler, node_connecting_program);
                }
                ntask::xend(scheduler);
            }
            ntask::xend(scheduler);

            // When the program is fully connected, we run this program that
            // checks if we are still connected both to WiFi and the Remote.
            // If not we jump to the connecting program to reconnect.
            // If both are connected we also call the main program.
            ntask::xbegin(scheduler, node_connected_program);
            {
                ntask::xif(scheduler, func_wifi_is_connected);
                {
                    ntask::xif(scheduler, func_remote_is_connected);
                    {
                        ntask::xrun(scheduler, main);
                        ntask::xreturn(scheduler);
                    }
                    ntask::xend(scheduler);

                    ntask::xjump(scheduler, node_connecting_program);
                }
                ntask::xend(scheduler);

                ntask::xjump(scheduler, node_connecting_program);
            }
            ntask::xend(scheduler);

            // The connecting program will first try to connect to WiFi, when connected
            // it will try to connect to the Remote. If both are connected we jump to
            // the connected program. If we time out trying to connect to either WiFi or Remote
            // we jump to the configure program to start over.
            ntask::xbegin(scheduler, node_connecting_program);
            {
                ntask::xonce(scheduler, func_connect_to_WiFi_start);
                ntask::xif(scheduler, func_wifi_is_connected);
                {
                    ntask::xonce(scheduler, func_connect_to_remote_start);

                    ntask::xif(scheduler, func_remote_connecting);
                    {
                        ntask::xjump(scheduler, node_connected_program);
                    }
                    ntask::xend(scheduler);

                    ntask::xif(scheduler, ntask::timeout_t(60000));
                    {
                        ntask::xonce(scheduler, func_wifi_disconnect);
                        ntask::xjump(scheduler, node_configure_program);
                    }
                    ntask::xend(scheduler);

                    ntask::xreturn(scheduler);
                }
                ntask::xend(scheduler);

                ntask::xif(scheduler, ntask::timeout_t(60000));
                {
                    ntask::xonce(scheduler, func_wifi_disconnect);
                    ntask::xjump(scheduler, node_configure_program);
                }
                ntask::xend(scheduler);
            }
            ntask::xend(scheduler);
        }

    }  // namespace nnode
}  // namespace ncore

#else

#    include "rdno_wifi/c_node.h"

namespace ncore
{
    namespace nnode
    {
        void connected(ntask::executor_t* scheduler, ntask::program_t main, ntask::state_t* state) { boot(scheduler, main); }

    }  // namespace nnode
}  // namespace ncore

#endif
