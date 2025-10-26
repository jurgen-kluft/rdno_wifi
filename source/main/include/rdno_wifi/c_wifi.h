#ifndef __RDNO_CORE_WIFI_H__
#define __RDNO_CORE_WIFI_H__
#include "rdno_core/c_target.h"
#ifdef USE_PRAGMA_ONCE
#    pragma once
#endif

#include "rdno_core/c_state.h"
#include "rdno_core/c_network.h"
#include "rdno_wifi/c_definitions.h"

namespace ncore
{
    namespace nwifi
    {
        void init_state(state_t* state);

        void connect(state_t* state);
        bool connected(state_t* state);
        void disconnect(state_t* state);

    }  // namespace nwifi
}  // namespace ncore

#endif  // __RDNO_CORE_WIFI_H__
