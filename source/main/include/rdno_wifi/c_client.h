#ifndef __RDNO_CORE_WIFI_CLIENT_H__
#define __RDNO_CORE_WIFI_CLIENT_H__
#include "rdno_core/c_target.h"
#ifdef USE_PRAGMA_ONCE
#    pragma once
#endif

#include "rdno_wifi/c_wifi.h"

namespace ncore
{
    namespace nclient
    {
        // @see: https://docs.arduino.cc/libraries/wifi/#Client%20class

        bool              NewClient();
        nstatus::status_t Connect(IPAddress_t ip, u16 port);
        nstatus::status_t Connect(IPAddress_t ip, u16 port, s32 timeout_ms);
        nstatus::status_t Connect(const char *host, u16 port);
        nstatus::status_t Connect(const char *host, u16 port, s32 timeout_ms);
        uint_t            Write(u8 data);
        uint_t            Write(const u8 *buf, uint_t size);

        nstatus::status_t Connected();
        s32               Available();
        s32               Read();
        s32               Read(u8 *buf, uint_t size);
        uint_t            ReadBytes(char *buffer, uint_t length);
        uint_t            ReadBytes(u8 *buffer, uint_t length);
        s32               Peek();
        void              Clear();  // clear rx
        void              Stop();
        void              SetSSE(bool sse);
        bool              IsSSE();

        s32  SetSocketOption(s32 option, char *value, uint_t len);
        s32  SetSocketOption(s32 level, s32 option, const void *value, uint_t len);
        s32  GetSocketOption(s32 level, s32 option, const void *value, uint_t size);
        s32  SetOption(s32 option, s32 *value);
        s32  GetOption(s32 option, s32 *value);
        void SetConnectionTimeout(u32 milliseconds);
        s32  SetNoDelay(bool nodelay);
        bool GetNoDelay();

        IPAddress_t RemoteIP();
        u16         RemotePort();
        IPAddress_t LocalIP();
        u16         LocalPort();
    }  // namespace nclient
}  // namespace ncore

#endif  // __RDNO_CORE_WIFI_CLIENT_H__
