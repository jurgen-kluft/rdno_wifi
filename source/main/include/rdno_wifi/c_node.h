#ifndef __RDNO_CORE_NODE_H__
#define __RDNO_CORE_NODE_H__
#include "rdno_core/c_target.h"
#ifdef USE_PRAGMA_ONCE
#    pragma once
#endif

#include "rdno_core/c_network.h"
#include "rdno_core/c_config.h"
#include "rdno_core/c_task.h"

namespace ncore
{
    namespace nnode
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

        // void node_setup(nconfig::config_t* config, s16 (*nameToIndex)(str_t const& str));
        // bool node_loop(nconfig::config_t* config, s16 (*nameToIndex)(str_t const& str));
        // u64 node_timesync();

        void schedule_connect(ntask::scheduler_t* scheduler, ntask::function_t onConnected, ntask::state_t* state);

    }  // namespace nnode
}  // namespace ncore

#endif  // __RDNO_CORE_NODE_H__
