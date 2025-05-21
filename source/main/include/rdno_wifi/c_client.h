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

    namespace nwifi
    {
        // @see: https://docs.arduino.cc/libraries/wifi/#Client%20class

        void InitMaxTcpClients(alloc_t *allocator, u16 max_clients);
        s16  NewTcpClient(alloc_t *allocator);

        s32    connect(s16 clientIndex, IPAddress_t ip, u16 port);
        s32    connect(s16 clientIndex, IPAddress_t ip, u16 port, s32 timeout_ms);
        s32    connect(s16 clientIndex, const char *host, u16 port);
        s32    connect(s16 clientIndex, const char *host, u16 port, s32 timeout_ms);
        uint_t write(s16 clientIndex, u8 data);
        uint_t write(s16 clientIndex, const u8 *buf, uint_t size);

        s32    available(s16 clientIndex);
        s32    read(s16 clientIndex);
        s32    read(s16 clientIndex, u8 *buf, uint_t size);
        uint_t readBytes(s16 clientIndex, char *buffer, uint_t length);
        uint_t readBytes(s16 clientIndex, u8 *buffer, uint_t length);
        s32    peek(s16 clientIndex);
        void   clear(s16 clientIndex);  // clear rx
        void   stop(s16 clientIndex);
        u8     connected(s16 clientIndex);
        void   setSSE(s16 clientIndex, bool sse);
        bool   isSSE(s16 clientIndex);

        s32  setSocketOption(s16 clientIndex, s32 option, char *value, uint_t len);
        s32  setSocketOption(s16 clientIndex, s32 level, s32 option, const void *value, uint_t len);
        s32  getSocketOption(s16 clientIndex, s32 level, s32 option, const void *value, uint_t size);
        s32  setOption(s16 clientIndex, s32 option, s32 *value);
        s32  getOption(s16 clientIndex, s32 option, s32 *value);
        void setConnectionTimeout(s16 clientIndex, u32 milliseconds);
        s32  setNoDelay(s16 clientIndex, bool nodelay);
        bool getNoDelay(s16 clientIndex);

        IPAddress_t remoteIP(s16 clientIndex);
        IPAddress_t remoteIP(s16 clientIndex, s32 fd);
        u16         remotePort(s16 clientIndex);
        u16         remotePort(s16 clientIndex, s32 fd);
        IPAddress_t localIP(s16 clientIndex);
        IPAddress_t localIP(s16 clientIndex, s32 fd);
        u16         localPort(s16 clientIndex);
        u16         localPort(s16 clientIndex, s32 fd);
    }  // namespace nwifi
}  // namespace ncore

#endif  // __RDNO_CORE_WIFI_CLIENT_H__
