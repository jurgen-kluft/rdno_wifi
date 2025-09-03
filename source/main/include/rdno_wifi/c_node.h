#ifndef __RDNO_CORE_WIFI_H__
#define __RDNO_CORE_WIFI_H__
#include "rdno_core/c_target.h"
#ifdef USE_PRAGMA_ONCE
#    pragma once
#endif

#include "rdno_core/c_network.h"
#include "rdno_core/c_nvstore.h"

namespace ncore
{
    namespace nwifi
    {
        namespace nparam
        {
            enum Enum
            {
                PARAM_WIFI_SSID            = 0,
                PARAM_WIFI_PASSWORD        = 1,
                PARAM_AP_SSID              = 2,
                PARAM_AP_PASSWORD          = 3,
                PARAM_REMOTE_SERVER_IP     = 4,
                PARAM_REMOTE_SERVER_PORT   = 5,
                PARAM_SENSOR_READ_INTERVAL = 6,
                PARAM_SENSOR_SEND_INTERVAL = 7,
            };
        }

        // UpdateConfig updates the WiFi configuration by receiving and parsing TCP packets
        // from the AP access point.
        void node_update(nvstore::config_t* config, s16 (*nameToIndex)(str_t const& str));
        void node_setup(nvstore::config_t* config, s16 (*nameToIndex)(str_t const& str));
        void node_loop(nvstore::config_t* config, s16 (*nameToIndex)(str_t const& str));

    }  // namespace nwifi
}  // namespace ncore

#endif  // __RDNO_CORE_WIFI_H__
