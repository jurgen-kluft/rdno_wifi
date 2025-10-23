#ifdef TARGET_ARDUINO

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

        ntask::result_t func_configure_start(ntask::state_t* state)
        {
            nwifi::set_mode_AP();

            // Start the access point to receive configuration
            char  ap_ssid_bytes[40];
            str_t ap_ssid = str_mutable(ap_ssid_bytes, sizeof(ap_ssid_bytes));
            str_append(ap_ssid, str_const("esp-"));
            MACAddress_t mac = nwifi::mac_address();
            to_str(ap_ssid, mac);

            str_t ap_password = str_const("1234");

            nwifi::begin_AP(ap_ssid.m_const, ap_password.m_const);
            nserial::printf("AP started on %s to receive configuration\n", va_t(ap_ssid.m_const));

            // Start the UDP socket to receive configuration data
            nudp::open(state->udp->m_instance, 4242);

            return ntask::RESULT_DONE;
        }

        char            gUdpMsgBytes[1500];
        ntask::result_t func_configure_loop(ntask::state_t* state)
        {
            IPAddress_t from_address;
            u16         from_port  = 0;
            const s32   udpMsgSize = nudp::receive(state->udp->m_instance, (byte*)gUdpMsgBytes, sizeof(gUdpMsgBytes), from_address, from_port);
            if (udpMsgSize > 0)
            {
                str_t msg = str_const_n(gUdpMsgBytes, udpMsgSize);
                if (str_len(msg) > 0)
                {
#    ifdef TARGET_DEBUG
                    nserial::println("Access Point, received message from connected client");
#    endif
                    str_t outKey;
                    str_t outValue;
                    while (nconfig::parse_keyvalue(msg, outKey, outValue))
                    {
                        // const s16 index = nameToIndex(outKey);
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

                        nserial::println("Access point, new configuration approved.");
                        nvstore::save(state->config);  // Save the configuration to non-volatile storage

                        return ntask::RESULT_DONE;
                    }
                    else
                    {
                        nserial::println("Access point, invalid configuration received, waiting for new configuration.");
                    }
                }
            }
            return ntask::RESULT_OK;
        }

        // We have a UDP socket that can receive configuration updates while connected
        ntask::result_t func_check_config_update(ntask::state_t* state)
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
            ntcp::disconnect(state->tcp, state->wifi->tcp_client);  // Stop any existing client connection

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
            nserial::printf("Connecting to %x:%d (IP: %d.%d.%d.%d) ...\n", va_t(remote_server), va_t(remote_port), va_t(remote_server_ip_address.m_address[0]), va_t(remote_server_ip_address.m_address[1]), va_t(remote_server_ip_address.m_address[2]),
                            va_t(remote_server_ip_address.m_address[3]));
#    endif

            state->wifi->tcp_client = ntcp::connect(state->tcp, remote_server_ip_address, remote_port, 8000);
            if (ntcp::connected(state->tcp, state->wifi->tcp_client) == nstatus::Connected)
            {
#    ifdef TARGET_DEBUG
                nserial::println("  -> Connected to remote.");

                IPAddress_t localIP = ntcp::local_IP(state->tcp, state->wifi->tcp_client);
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

            return ntask::RESULT_OK;
        }

        ntask::result_t func_remote_is_connected(ntask::state_t* state)
        {
            if (ntcp::connected(state->tcp, state->wifi->tcp_client) == nstatus::Connected)
            {
                return ntask::RESULT_DONE;
            }

            return ntask::RESULT_OK;
        }

        void initialize(ntask::executor_t* scheduler, ntask::program_t main, ntask::state_t* state)
        {
            ntask::program_t node_configure_program  = ntask::program(scheduler, "configuration program");
            ntask::program_t node_connected_program  = ntask::program(scheduler, "connected program");
            ntask::program_t node_connecting_program = ntask::program(scheduler, "connecting program");

            // Boot by jumping to the connecting program if we have valid configuration, otherwise
            // jump to the configuration program to setup WiFi and Remote configuration.
            ntask::timeout_t config_timeout(1 * 60 * 1000);
            if (state->has_config())
            {
                ntask::boot(scheduler, node_connecting_program);
            }
            else
            {
                ntask::boot(scheduler, node_configure_program);
                config_timeout.m_timeout_ms = 0;  // No timeout if we don't have configuration
            }

            // The configuration program will start an access point and UDP to receive configuration data.
            // When valid configuration is received it will jump to the connecting program, otherwise it will
            // keep waiting for valid configuration.
            // Once the connecting program is started it will try to connect to WiFi and the Remote, when both
            // are connected it will jump to the connected program.
            // The connected program will check if we are still connected to both WiFi and Remote, and will
            // call the main program. If either WiFi or Remote are disconnected it will jump to the connecting
            // program to reconnect.

            // TODO Do we need a timeout on this program, that no matter what we jump to the connecting program
            //      to try to connect to WiFi and Remote?
            ntask::op_begin(scheduler, node_configure_program);
            {
                ntask::op_once(scheduler, func_configure_start);
                ntask::op_if(scheduler, func_configure_loop);
                {
                    ntask::op_jump(scheduler, node_connecting_program);
                }
                ntask::op_end(scheduler);

                // If we time out while waiting for configuration, we jump to the connecting program
                // Note: Currently set to 5 minutes
                ntask::op_if(scheduler, config_timeout);
                {
                    ntask::op_jump(scheduler, node_connecting_program);
                }
                ntask::op_end(scheduler);
            }
            ntask::op_end(scheduler);

            // When the program is fully connected, we run this program that
            // checks if we are still connected both to WiFi and the Remote.
            // If not we jump to the connecting program to reconnect.
            // If both are connected we also call the main program.
            ntask::op_begin(scheduler, node_connected_program);
            {
                ntask::op_if(scheduler, func_wifi_is_connected);
                {
                    ntask::op_if(scheduler, func_remote_is_connected);
                    {
                        ntask::op_run(scheduler, main);
                        ntask::op_return(scheduler);
                    }
                    ntask::op_end(scheduler);

                    ntask::op_jump(scheduler, node_connecting_program);
                }
                ntask::op_end(scheduler);

                ntask::op_jump(scheduler, node_connecting_program);
            }
            ntask::op_end(scheduler);

            // The connecting program will first try to connect to WiFi, when connected
            // it will try to connect to the Remote. If both are connected we jump to
            // the connected program. If we time out trying to connect to either WiFi or Remote
            // we jump to the configure program to start over.
            ntask::op_begin(scheduler, node_connecting_program);
            {
                ntask::op_once(scheduler, func_connect_to_WiFi_start);
                ntask::op_if(scheduler, func_wifi_is_connected);
                {
                    ntask::op_once(scheduler, func_connect_to_remote_start);

                    ntask::op_if(scheduler, func_remote_connecting);
                    {
                        ntask::op_jump(scheduler, node_connected_program);
                    }
                    ntask::op_end(scheduler);

                    ntask::op_if(scheduler, ntask::timeout_t(300 * 1000));
                    {
                        ntask::op_once(scheduler, func_wifi_disconnect);
                        ntask::op_jump(scheduler, node_configure_program);
                    }
                    ntask::op_end(scheduler);

                    ntask::op_return(scheduler);
                }
                ntask::op_end(scheduler);

                ntask::op_if(scheduler, ntask::timeout_t(300 * 1000));
                {
                    ntask::op_once(scheduler, func_wifi_disconnect);
                    ntask::op_jump(scheduler, node_configure_program);
                }
                ntask::op_end(scheduler);
            }
            ntask::op_end(scheduler);
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
