#ifndef __RDNO_CORE_NODE_H__
#define __RDNO_CORE_NODE_H__
#include "rdno_core/c_target.h"
#ifdef USE_PRAGMA_ONCE
#    pragma once
#endif

namespace ncore
{
    struct state_t;
    struct state_task_t;

    namespace nnode
    {
        void initialize(state_t* state, state_task_t* task_state);

    }  // namespace nnode
}  // namespace ncore

#endif  // __RDNO_CORE_NODE_H__
