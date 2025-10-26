#ifndef __RDNO_CORE_WIFI_TCP_H__
#define __RDNO_CORE_WIFI_TCP_H__
#include "rdno_core/c_target.h"
#ifdef USE_PRAGMA_ONCE
#    pragma once
#endif

#include "rdno_core/c_network.h"
#include "rdno_wifi/c_definitions.h"

namespace ncore
{
    struct state_t;
    struct state_tcp_t;

    namespace ntcp
    {
        void        init_state(state_t* state);
        client_t    connect(state_tcp_t* state, IPAddress_t ip, u16 port, s32 timeout_ms);
        bool        disconnect(state_tcp_t* state, client_t& client);
        s32         write(state_tcp_t* state, client_t client, const u8* buf, s32 size);
        bool        connected(state_tcp_t* state, client_t client);
        s32         available(state_tcp_t* state, client_t client);
        IPAddress_t remote_IP(state_tcp_t* state, client_t client);
        u16         remote_port(state_tcp_t* state, client_t client);
        IPAddress_t local_IP(state_tcp_t* state, client_t client);
        u16         local_port(state_tcp_t* state, client_t client);
    }  // namespace ntcp

}  // namespace ncore

#endif  // __RDNO_CORE_WIFI_TCP_H__
