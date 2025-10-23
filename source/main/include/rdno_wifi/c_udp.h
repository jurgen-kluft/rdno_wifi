#ifndef __RDNO_CORE_WIFI_UDP_H__
#define __RDNO_CORE_WIFI_UDP_H__
#include "rdno_core/c_target.h"
#ifdef USE_PRAGMA_ONCE
#    pragma once
#endif

#include "rdno_wifi/c_definitions.h"

namespace ncore
{
    namespace nudp
    {
        struct sock_t
        {
            sock_t()
                : m_fd(-1)
                , m_port(0)
            {
            }
            s32 m_fd;
            u16 m_port;
        };

        bool open(sock_t& instance, u16 port);
        void close(sock_t& instance);
        s32  receive(sock_t& instance, byte* data, s32 max_data_size, IPAddress_t &remote_ip, u16 &remote_port);
        s32  send_to(sock_t& instance, byte const* data, s32 data_size, const IPAddress_t& to_address, u16 to_port);
    }  // namespace nudp

    struct state_udp_t
    {
        nudp::sock_t m_instance;
        state_udp_t()
            : m_instance{}
        {
        }
    };

}  // namespace ncore

#endif  // __RDNO_CORE_WIFI_UDP_H__
