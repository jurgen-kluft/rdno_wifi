#ifndef __RDNO_CORE_WIFI_CONNECT_TO_REMOTE_H__
#define __RDNO_CORE_WIFI_CONNECT_TO_REMOTE_H__
#include "rdno_core/c_target.h"
#ifdef USE_PRAGMA_ONCE
#    pragma once
#endif

#include "rdno_wifi/c_wifi.h"

namespace ncore
{
    namespace nremote
    {
        // Connecting to a remote server
        // @see: https://docs.arduino.cc/libraries/wifi/#Client%20class

        nstatus::status_t connect(IPAddress_t ip, u16 port);
        nstatus::status_t connect(IPAddress_t ip, u16 port, s32 timeout_ms);
        nstatus::status_t connect(const char *host, u16 port);
        nstatus::status_t connect(const char *host, u16 port, s32 timeout_ms);
        uint_t            write(u8 data);
        uint_t            write(const u8 *buf, uint_t size);

        nstatus::status_t connected();
        s32               available();
        s32               read();
        s32               read(u8 *buf, uint_t size);
        uint_t            read_bytes(char *buffer, uint_t length);
        uint_t            read_bytes(u8 *buffer, uint_t length);
        s32               peek();
        void              clear();  // clear rx
        void              stop();
        void              set_SSE(bool sse);
        bool              is_SSE();

        s32  set_socket_option(s32 option, char *value, uint_t len);
        s32  set_socket_option(s32 level, s32 option, const void *value, uint_t len);
        s32  get_socket_option(s32 level, s32 option, const void *value, uint_t size);
        s32  set_option(s32 option, s32 *value);
        s32  get_option(s32 option, s32 *value);
        void set_connection_timeout(u32 milliseconds);
        s32  set_no_delay(bool nodelay);
        bool get_no_delay();

        IPAddress_t remote_IP();
        u16         remote_port();
        IPAddress_t local_IP();
        u16         local_port();
    }  // namespace nremote
}  // namespace ncore

#endif  // __RDNO_CORE_WIFI_CLIENT_H__
