#    include "rdno_wifi/c_client.h"

#ifdef TARGET_ESP32

#    include "Arduino.h"
#    include "WiFiClient.h"

namespace ncore
{
    namespace nremote
    {
        ncore::s16 gNumClients = 0;
        WiFiClient gWiFiClient;

        nstatus::status_t connect(IPAddress_t _ip, u16 _port)
        {
            IPAddress ip(_ip.m_address[0], _ip.m_address[1], _ip.m_address[2], _ip.m_address[3]);
            if (gWiFiClient.connect(ip, _port) == 0)
                return nstatus::ConnectFailed;
            return nstatus::Connected;
        }
        nstatus::status_t connect(IPAddress_t _ip, u16 _port, s32 timeout_ms)
        {
            IPAddress ip(_ip.m_address[0], _ip.m_address[1], _ip.m_address[2], _ip.m_address[3]);
            if (gWiFiClient.connect(ip, _port, timeout_ms) == 0)
                return nstatus::ConnectFailed;
            return nstatus::Connected;
        }
        nstatus::status_t connect(const char *host, u16 port)
        {
            if (gWiFiClient.connect(host, port) == 0)
                return nstatus::ConnectFailed;
            return nstatus::Connected;
        }
        nstatus::status_t connect(const char *host, u16 port, s32 timeout_ms)
        {
            if (gWiFiClient.connect(host, port, timeout_ms) == 0)
                return nstatus::ConnectFailed;
            return nstatus::Connected;
        }
        uint_t write(u8 data) { return gWiFiClient.write(data); }
        uint_t write(const u8 *buf, uint_t size) { return gWiFiClient.write(buf, size); }

        nstatus::status_t connected()
        {
            if (gWiFiClient.connected() == 0)
                return nstatus::Disconnected;
            return nstatus::Connected;
        }
        s32    available() { return gWiFiClient.available(); }
        s32    read() { return gWiFiClient.read(); }
        s32    read(u8 *buf, uint_t size) { return gWiFiClient.read(buf, size); }
        uint_t read_bytes(char *buffer, uint_t length) { return gWiFiClient.readBytes(buffer, length); }
        uint_t read_bytes(u8 *buffer, uint_t length) { return gWiFiClient.readBytes(buffer, length); }
        s32    peek() { return gWiFiClient.peek(); }
        void   clear() { gWiFiClient.clear(); }
        void   stop() { gWiFiClient.stop(); }
        void   set_SSE(bool sse) { gWiFiClient.setSSE(sse); }
        bool   is_SSE() { return gWiFiClient.isSSE(); }

        s32  set_socket_option(s32 option, char *value, uint_t len) { return gWiFiClient.setSocketOption(option, value, len); }
        s32  set_socket_option(s32 level, s32 option, const void *value, uint_t len) { return gWiFiClient.setSocketOption(level, option, value, len); }
        s32  get_socket_option(s32 level, s32 option, const void *value, uint_t size) { return gWiFiClient.getSocketOption(level, option, value, size); }
        s32  set_option(s32 option, s32 *value) { return gWiFiClient.setOption(option, value); }
        s32  get_option(s32 option, s32 *value) { return gWiFiClient.getOption(option, value); }
        void set_connection_timeout(u32 milliseconds) { gWiFiClient.setConnectionTimeout(milliseconds); }
        s32  set_no_delay(bool nodelay) { return gWiFiClient.setNoDelay(nodelay); }
        bool get_no_delay() { return gWiFiClient.getNoDelay(); }

        IPAddress_t remote_IP()
        {
            IPAddress ip = gWiFiClient.remoteIP();
            IPAddress_t rip;
            rip.m_address[0] = ip[0];
            rip.m_address[1] = ip[1];
            rip.m_address[2] = ip[2];
            rip.m_address[3] = ip[3];
            return rip;
        }
        u16         remote_port() { return gWiFiClient.remotePort(); }
        IPAddress_t local_IP()
        {
            IPAddress ip = gWiFiClient.localIP();
            IPAddress_t lip;
            lip.m_address[0] = ip[0];
            lip.m_address[1] = ip[1];
            lip.m_address[2] = ip[2];
            lip.m_address[3] = ip[3];
            return lip;
        }
        u16 local_port() { return gWiFiClient.localPort(); }

    }  // namespace nclient
}  // namespace ncore

#else

namespace ncore
{
    namespace nremote
    {
        nstatus::status_t connect(IPAddress_t ip, u16 port) { return nstatus::Connected; }
        nstatus::status_t connect(IPAddress_t ip, u16 port, s32 timeout_ms) { return nstatus::Connected; }
        nstatus::status_t connect(const char *host, u16 port) { return nstatus::Connected; }
        nstatus::status_t connect(const char *host, u16 port, s32 timeout_ms) { return nstatus::Connected; }
        uint_t            write(u8 data) { return 1; }
        uint_t            write(const u8 *buf, uint_t size) { return size; }

        s32               available() { return 1; }
        s32               read() { return 1; }
        s32               read(u8 *buf, uint_t size) { return 1; }
        uint_t            read_bytes(char *buffer, uint_t length) { return 1; }
        uint_t            read_bytes(u8 *buffer, uint_t length) { return 1; }
        s32               peek() { return 0; }
        void              clear() {}
        void              stop() {}
        nstatus::status_t connected() { return nstatus::Connected; }
        void              set_SSE(bool sse) {}
        bool              is_SSE() { return false; }

        s32  set_socket_option(s32 option, char *value, uint_t len) { return 0; }
        s32  set_socket_option(s32 level, s32 option, const void *value, uint_t len) { return 0; }
        s32  get_socket_option(s32 level, s32 option, const void *value, uint_t size) { return 0; }
        s32  set_option(s32 option, s32 *value) { return 0; }
        s32  get_option(s32 option, s32 *value) { return 0; }
        void set_connection_timeout(u32 milliseconds) {}
        s32  set_no_delay(bool nodelay) { return 0; }
        bool get_no_delay() { return false; }

        IPAddress_t remote_IP() { return IPAddress_t{10, 0, 0, 43}; }
        u16         remote_port() { return 4242; }
        IPAddress_t local_IP() { return IPAddress_t{10, 0, 0, 42}; }
        u16         local_port() { return 4242; }

    }  // namespace nclient
}  // namespace ncore

#endif
