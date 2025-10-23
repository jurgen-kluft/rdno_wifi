#ifndef __RDNO_CORE_WIFI_DEFINITIONS_H__
#define __RDNO_CORE_WIFI_DEFINITIONS_H__
#include "rdno_core/c_target.h"
#ifdef USE_PRAGMA_ONCE
#    pragma once
#endif

#include "rdno_core/c_network.h"

namespace ncore
{
    namespace nconfig
    {
        struct config_t;
    }

    namespace ntcp
    {
        typedef void* client_t;
    }

    struct state_wifi_t
    {
        ntcp::client_t tcp_client;
    };

    namespace nstatus
    {
        typedef s32    status_t;
        const status_t Idle            = 0;
        const status_t NoSSIDAvailable = 1;
        const status_t ScanCompleted   = 2;
        const status_t Connected       = 3;
        const status_t ConnectFailed   = 4;
        const status_t ConnectionLost  = 5;
        const status_t Disconnected    = 6;
        const status_t Stopped         = 254;
        const status_t NoShield        = 255;
    }  // namespace nstatus

    namespace nwifi
    {
        // @see: https://www.arduino.cc/en/Reference/WiFi

        struct BSID_t
        {
            byte mB0, mB1, mB2, mB3, mB4, mB5, mB6, mB7;
        };
    }  // namespace nwifi
}  // namespace ncore

#endif  // __RDNO_CORE_WIFI_DEFINITIONS_H__
