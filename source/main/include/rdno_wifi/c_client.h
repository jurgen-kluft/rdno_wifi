#ifndef __RDNO_CORE_WIFI_CLIENT_H__
#define __RDNO_CORE_WIFI_CLIENT_H__
#include "rdno_core/c_target.h"
#ifdef USE_PRAGMA_ONCE
#    pragma once
#endif

#include "rdno_wifi/c_wifi.h"

namespace ncore
{
    class alloc_t;

    namespace nclient
    {
        // @see: https://docs.arduino.cc/libraries/wifi/#Client%20class

        void InitMaxClients(alloc_t *allocator, u16 max_clients);
        s16  NewClient(alloc_t *allocator);

        s32    Connect(s16 clientIndex, IPAddress_t ip, u16 port);
        s32    Connect(s16 clientIndex, IPAddress_t ip, u16 port, s32 timeout_ms);
        s32    Connect(s16 clientIndex, const char *host, u16 port);
        s32    Connect(s16 clientIndex, const char *host, u16 port, s32 timeout_ms);
        uint_t Write(s16 clientIndex, u8 data);
        uint_t Write(s16 clientIndex, const u8 *buf, uint_t size);

        s32    Available(s16 clientIndex);
        s32    Read(s16 clientIndex);
        s32    Read(s16 clientIndex, u8 *buf, uint_t size);
        uint_t ReadBytes(s16 clientIndex, char *buffer, uint_t length);
        uint_t ReadBytes(s16 clientIndex, u8 *buffer, uint_t length);
        s32    Peek(s16 clientIndex);
        void   Clear(s16 clientIndex);  // clear rx
        void   Stop(s16 clientIndex);
        u8     Connected(s16 clientIndex);
        void   SetSSE(s16 clientIndex, bool sse);
        bool   IsSSE(s16 clientIndex);

        s32  SetSocketOption(s16 clientIndex, s32 option, char *value, uint_t len);
        s32  SetSocketOption(s16 clientIndex, s32 level, s32 option, const void *value, uint_t len);
        s32  GetSocketOption(s16 clientIndex, s32 level, s32 option, const void *value, uint_t size);
        s32  SetOption(s16 clientIndex, s32 option, s32 *value);
        s32  GetOption(s16 clientIndex, s32 option, s32 *value);
        void SetConnectionTimeout(s16 clientIndex, u32 milliseconds);
        s32  SetNoDelay(s16 clientIndex, bool nodelay);
        bool GetNoDelay(s16 clientIndex);

        IPAddress_t RemoteIP(s16 clientIndex);
        IPAddress_t RemoteIP(s16 clientIndex, s32 fd);
        u16         RemotePort(s16 clientIndex);
        u16         RemotePort(s16 clientIndex, s32 fd);
        IPAddress_t LocalIP(s16 clientIndex);
        IPAddress_t LocalIP(s16 clientIndex, s32 fd);
        u16         LocalPort(s16 clientIndex);
        u16         LocalPort(s16 clientIndex, s32 fd);
    }  // namespace nwifi
}  // namespace ncore

#endif  // __RDNO_CORE_WIFI_CLIENT_H__
