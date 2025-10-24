#ifdef TARGET_ARDUINO

#    include "Arduino.h"
#    include "WiFi.h"

#    include "rdno_core/c_app.h"
#    include "rdno_core/c_config.h"
#    include "rdno_core/c_packet.h"
#    include "rdno_core/c_network.h"
#    include "rdno_core/c_nvstore.h"
#    include "rdno_core/c_serial.h"
#    include "rdno_core/c_state.h"
#    include "rdno_core/c_str.h"
#    include "rdno_core/c_task.h"
#    include "rdno_core/c_timer.h"

#    include "rdno_wifi/c_tcp.h"
#    include "rdno_wifi/c_udp.h"
#    include "rdno_wifi/c_wifi.h"
#    include "rdno_wifi/c_node.h"

namespace ncore
{
    namespace nnode
    {
        // ----------------------------------------------------------------
        // ----------------------------------------------------------------
        // ntask based version
        // ----------------------------------------------------------------
        // ----------------------------------------------------------------

        ntask::result_t func_configure_start(state_t* state)
        {
            nwifi::set_mode_AP();

            // Start the access point to receive configuration
            char  ap_ssid_bytes[40];
            str_t ap_ssid = str_mutable(ap_ssid_bytes, sizeof(ap_ssid_bytes));
            str_append(ap_ssid, "esp-");
            MACAddress_t mac = nwifi::mac_address();
            str_append(ap_ssid, mac);
            str_t ap_password = str_const("1234");

            nwifi::begin_AP(ap_ssid.m_const, ap_password.m_const);
            nserial::printf("AP started on %s to receive configuration\n", va_t(ap_ssid.m_const));

            // Start the UDP socket to receive configuration data
            nudp::open(state->node->udp_socket, 42420);

            return ntask::RESULT_DONE;
        }

        char            gUdpMsgBytes[1500];
        ntask::result_t func_configure_loop(state_t* state)
        {
            IPAddress_t from_address;
            u16         from_port = 0;

            const s32 udpMsgSize = nudp::receive(state->node->udp_socket, (byte*)gUdpMsgBytes, sizeof(gUdpMsgBytes), from_address, from_port);
            if (udpMsgSize > 0)
            {
                str_t msg = str_const_n(gUdpMsgBytes, udpMsgSize);
                if (str_len(msg) > 0)
                {
#    ifdef TARGET_DEBUG
                    nserial::println("Access Point, received configuration");
#    endif
                    str_t outKey;
                    str_t outValue;
                    while (nconfig::parse_keyvalue(msg, outKey, outValue))
                    {
                        const s16 index = napp::config_key_to_index(outKey);
#    ifdef TARGET_DEBUG
                        nserial::printf("Parse %s = %s, index = %d\n", va_t(outKey.m_const + outKey.m_str), va_t(outValue.m_const + outValue.m_str), va_t(index));
#    endif
                        nconfig::parse_value(state->config, index, outValue);
                    }
                    str_clear(msg);

                    // Check if we have valid WiFi configuration
                    str_t ssid          = str_empty();
                    str_t pass          = str_empty();
                    u32   remote_server = 0;
                    u16   remote_port   = 0;

                    nconfig::get_string(state->config, nconfig::PARAM_ID_WIFI_SSID, ssid);
                    nconfig::get_string(state->config, nconfig::PARAM_ID_WIFI_PASSWORD, pass);
                    nconfig::get_uint32(state->config, nconfig::PARAM_ID_REMOTE_IP, remote_server);
                    nconfig::get_uint16(state->config, nconfig::PARAM_ID_REMOTE_PORT, remote_port);

#    ifdef TARGET_DEBUG
                    char        remote_server_str_buffer[24];
                    str_t       remote_server_str = str_mutable(remote_server_str_buffer, sizeof(remote_server_str_buffer));
                    IPAddress_t remote_server_ipaddress;
                    remote_server_ipaddress.from(remote_server);
                    to_str(remote_server_str, remote_server_ipaddress);

                    nserial::println("Access point, new configuration:");
                    nserial::printfln("    WiFi SSID: %s", va_t(ssid.m_const + ssid.m_str));
                    nserial::printfln("           PW: %s", va_t(pass.m_const + pass.m_str));
                    nserial::printfln("    SERVER IP: %s", va_t(remote_server_str.m_const));
                    nserial::printfln("         PORT: %d", va_t(remote_port));
#    endif
                    const bool valid_wifi_config   = is_valid_SSID(ssid) && is_valid_password(pass);
                    const bool valid_server_config = is_valid_IPAddress(remote_server) && is_valid_port(remote_port);
                    if (valid_wifi_config && valid_server_config)
                    {
                        nwifi::disconnect_AP(false);  // Stop the access point

                        nserial::println("Access point, new configuration");
                        nvstore::save(state->config);  // Save the configuration to non-volatile storage

                        return ntask::RESULT_DONE;
                    }
                    else
                    {
                        nserial::println("Access point, configuration invalid!");
                    }
                }
            }
            return ntask::RESULT_OK;
        }

        // We have a UDP socket that can receive configuration updates while connected
        ntask::result_t func_check_config_update(state_t* state)
        {
            if (func_configure_loop(state) == ntask::RESULT_DONE)
            {
                nserial::println("Configuration updated, restarting to apply new configuration.");
                nvstore::save(state->config);  // Save the configuration to non-volatile storage
                // esp_restart();                 // Restart the system to apply new configuration
                return ntask::RESULT_DONE;
            }
            return ntask::RESULT_OK;
        }

        ntask::result_t func_connect_to_WiFi_start(state_t* state)
        {
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

        ntask::result_t func_wifi_is_connected(state_t* state)
        {
            if (nwifi::status() == nstatus::Connected)
                return ntask::RESULT_DONE;
            return ntask::RESULT_OK;
        }

        ntask::result_t func_wifi_disconnect(state_t* state)
        {
            nwifi::disconnect();
            return ntask::RESULT_DONE;
        }

        ntask::result_t func_connect_to_remote_start(state_t* state)
        {
            ntcp::disconnect(state->tcp, state->node->tcp_client);  // Stop any existing client connection

            if (nwifi::status() != nstatus::Connected)
            {
                nserial::println("  -> Connecting to remote stopped, WiFi not connected.");
                return ntask::RESULT_ERROR;
            }

            nserial::println("Connecting to remote.");
            return ntask::RESULT_DONE;
        }

        ntask::result_t func_remote_connecting(state_t* state)
        {
            nconfig::config_t* config = state->config;

            u32 remote_server;
            if (!nconfig::get_uint32(config, nconfig::PARAM_ID_REMOTE_IP, remote_server))
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
            remote_server_ip_address.from(remote_server);

#    ifdef TARGET_DEBUG
            nserial::printf("Connecting to %x:%d (IP: %d.%d.%d.%d, MODE: %d) ...\n", va_t(remote_server), va_t(remote_port), va_t(remote_server_ip_address.m_address[0]), va_t(remote_server_ip_address.m_address[1]),
                            va_t(remote_server_ip_address.m_address[2]), va_t(state->node->remote_mode), va_t(remote_server_ip_address.m_address[3]));
#    endif
            if (state->node->remote_mode == 0)
            {
                state->node->tcp_client = ntcp::connect(state->tcp, remote_server_ip_address, remote_port, 8000);
                if (ntcp::connected(state->tcp, state->node->tcp_client) == nstatus::Connected)
                {
#    ifdef TARGET_DEBUG
                    nserial::println("  -> Connected to remote.");

                    IPAddress_t localIP = ntcp::local_IP(state->tcp, state->node->tcp_client);
                    nserial::print("     IP: ");
                    nserial::print(localIP);
                    nserial::println("");

                    MACAddress_t mac = nwifi::mac_address();
                    nserial::print("     MAC: ");
                    nserial::print(mac);
                    nserial::println("");
#    endif
                    state->time_ms = ntimer::millis();
                    // state->time_sync = ntimer::millis();

                    return ntask::RESULT_DONE;
                }
            }
            else if (state->node->remote_mode == 1)
            {
                return ntask::RESULT_DONE;
            }
            return ntask::RESULT_OK;
        }

        ntask::result_t func_remote_is_connected(state_t* state)
        {
            if (state->node->remote_mode == 1)
            {
                return ntask::RESULT_DONE;
            }
            else
            {
                if (ntcp::connected(state->tcp, state->node->tcp_client) == nstatus::Connected)
                {
                    return ntask::RESULT_DONE;
                }
            }
            return ntask::RESULT_OK;
        }

        void node_connecting_wifi(ntask::scheduler_t* scheduler, state_t* state);
        ntask::program_t program_node_connecting_wifi(node_connecting_wifi);

        void node_connected_tcp_program(ntask::scheduler_t* scheduler, state_t* state)
        {
            if (ntask::call(scheduler, func_wifi_is_connected))
            {
                if (ntask::call(scheduler, func_remote_is_connected))
                {
                    ntask::call(scheduler, func_check_config_update);
                    ntask::call_program(scheduler->m_state_task->m_main_program);
                    return;
                }
            }
            ntask::jmp_program(scheduler, &program_node_connecting_wifi);
        }
        ntask::program_t program_node_connected_tcp(node_connected_tcp_program);

        void node_connected_udp_program(ntask::scheduler_t* scheduler, state_t* state)
        {
            if (ntask::call(scheduler, func_wifi_is_connected))
            {
                ntask::call(scheduler, func_check_config_update);
                ntask::call_program(scheduler->m_state_task->m_main_program);
                return;
            }
            ntask::jmp_program(scheduler, &program_node_connecting_wifi);
        }
        ntask::program_t program_node_connected_udp(node_connected_udp_program);

        ntask::timeout_t gConfigureTimeout(5 * 60 * 1000);
        void node_configure_program(ntask::scheduler_t* scheduler, state_t* state)
        {
            // // If we time out while waiting for configuration, we jump to the connecting program
            // // Note: Currently set to 5 minutes
            if (ntask::is_first_call(scheduler))
            {
                ntask::init_timeout(scheduler, gConfigureTimeout);
                ntask::call(scheduler, func_configure_start);
            }

            if (ntask::call(scheduler, func_configure_loop))
            {
                ntask::jmp_program(scheduler, &program_node_connecting_wifi);
            }
            else if (ntask::timeout(scheduler, gConfigureTimeout))
            {
                ntask::jmp_program(scheduler, &program_node_connecting_wifi);
            }
        }
        ntask::program_t program_node_configure(node_configure_program);

        ntask::timeout_t gRemoteConnectTimeout(300 * 1000);
        void node_connecting_remote(ntask::scheduler_t* scheduler, state_t* state)
        {
            if (ntask::is_first_call(scheduler))
            {
                ntask::init_timeout(scheduler, gRemoteConnectTimeout);
                ntask::call(scheduler, func_connect_to_remote_start);
            }

            if (ntask::call(scheduler, func_remote_is_connected))
            {
                ntask::jmp_program(scheduler, &program_node_connected_tcp);
            }
            else if (ntask::timeout(scheduler, gRemoteConnectTimeout))
            {
                ntask::call(scheduler, func_wifi_disconnect);
                ntask::jmp_program(scheduler, &program_node_configure);
            }
        }
        ntask::program_t program_node_connecting_remote(node_connecting_remote);

        ntask::timeout_t gWiFiConnectTimeout(300 * 1000);
        void node_connecting_wifi(ntask::scheduler_t* scheduler, state_t* state)
        {
            if (ntask::is_first_call(scheduler))
            {
                ntask::init_timeout(scheduler, gWiFiConnectTimeout);
                ntask::call(scheduler, func_connect_to_WiFi_start);
            }

            if (ntask::call(scheduler, func_wifi_is_connected))
            {
                ntask::jmp_program(scheduler, &program_node_connecting_remote);
            }
            else if (ntask::timeout(scheduler, gWiFiConnectTimeout))
            {
                ntask::call(scheduler, func_wifi_disconnect);
                ntask::jmp_program(scheduler, &program_node_configure);
            }
        }

        state_node_t gNodeState;
        void         initialize(state_t* state, state_task_t* task_state)
        {
            nwifi::set_state(state);

            gNodeState.remote_mode = 1;  // Default to UDP
            nconfig::get_uint8(state->config, nconfig::PARAM_ID_REMOTE_MODE, gNodeState.remote_mode);
            gNodeState.tcp_client = nullptr;
            gNodeState.udp_socket = nullptr;
            state->node           = &gNodeState;

            if (state->has_config())
            {
                ntask::set_start(state, task_state, &program_node_connecting_wifi);
            }
            else
            {
                ntask::set_start(state, task_state, &program_node_configure);
            }
        }


    }  // namespace nnode
}  // namespace ncore

#else

#    include "rdno_wifi/c_node.h"

namespace ncore
{
    namespace nnode
    {
        void connected(ntask::scheduler_t* scheduler, ntask::program_t main, state_t* state) { boot(scheduler, main); }

    }  // namespace nnode
}  // namespace ncore

#endif
