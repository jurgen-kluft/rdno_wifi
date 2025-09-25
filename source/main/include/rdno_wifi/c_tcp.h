#ifndef __RDNO_CORE_WIFI_CLIENT_H__
#define __RDNO_CORE_WIFI_CLIENT_H__
#include "rdno_core/c_target.h"
#ifdef USE_PRAGMA_ONCE
#    pragma once
#endif

#include "rdno_wifi/c_wifi.h"

namespace ncore
{
    struct str_t;

    namespace ntcp
    {
        typedef void* client_t;

        void     server_start(u16 port);
        client_t server_handle_client();
        void     server_stop();

        bool     client_is_connected(client_t client);
        bool     client_recv_msg(client_t client, str_t& msg);

    }  // namespace ntcp
}  // namespace ncore

#endif  // __RDNO_CORE_WIFI_CLIENT_H__
