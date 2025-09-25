#ifndef __RDNO_CORE_NODE_H__
#define __RDNO_CORE_NODE_H__
#include "rdno_core/c_target.h"
#ifdef USE_PRAGMA_ONCE
#    pragma once
#endif

#include "rdno_core/c_task.h"

namespace ncore
{
    namespace nnode
    {
        void schedule_connect(ntask::executor_t* scheduler, ntask::program_t main, ntask::state_t* state);

    }  // namespace nnode
}  // namespace ncore

#endif  // __RDNO_CORE_NODE_H__
