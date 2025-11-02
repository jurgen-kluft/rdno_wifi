#ifndef __RDNO_CORE_WIFI_UDP_H__
#define __RDNO_CORE_WIFI_UDP_H__
#include "rdno_core/c_target.h"
#ifdef USE_PRAGMA_ONCE
#    pragma once
#endif

#include "rdno_wifi/c_definitions.h"

namespace ncore
{
    struct state_t;
    struct state_udp_t;

    namespace nudp
    {
        void init_state(state_t* state);

        bool open(state_t* state, u16 port);
        void close(state_t* state);
        s32  receive(state_t* state, byte* data, s32 max_data_size, IPAddress_t &remote_ip, u16 &remote_port);
        s32  send_to(state_t* state, byte const* data, s32 data_size, const IPAddress_t& to_address, u16 to_port);
    }  // namespace nudp

}  // namespace ncore

#endif  // __RDNO_CORE_WIFI_UDP_H__
