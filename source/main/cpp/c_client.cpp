#ifdef TARGET_ESP32

#    include "Arduino.h"
#    include "WiFiClient.h"

#    include "rdno_wifi/c_client.h"

ncore::s16 gNumClients = 0;
WiFiClient gWiFiClient;

namespace ncore
{
    namespace nclient
    {
        bool NewClient()
        {
            if (gNumClients == 0)
            {
                gNumClients++;
                return true;
            }
            return false;
        }

        nstatus::status_t Connect(IPAddress_t _ip, u16 _port)
        {
            IPAddress ip(_ip.m_address[0], _ip.m_address[1], _ip.m_address[2], _ip.m_address[3]);
            if (gWiFiClient.connect(ip, _port) == 0)
                return nstatus::ConnectFailed;
            return nstatus::Connected;
        }
        nstatus::status_t Connect(IPAddress_t _ip, u16 _port, s32 timeout_ms)
        {
            IPAddress ip(_ip.m_address[0], _ip.m_address[1], _ip.m_address[2], _ip.m_address[3]);
            if (gWiFiClient.connect(ip, _port, timeout_ms) == 0)
                return nstatus::ConnectFailed;
            return nstatus::Connected;
        }
        nstatus::status_t Connect(const char *host, u16 port)
        {
            if (gWiFiClient.connect(host, port) == 0)
                return nstatus::ConnectFailed;
            return nstatus::Connected;
        }
        nstatus::status_t Connect(const char *host, u16 port, s32 timeout_ms)
        {
            if (gWiFiClient.connect(host, port, timeout_ms) == 0)
                return nstatus::ConnectFailed;
            return nstatus::Connected;
        }
        uint_t Write(u8 data) { return gWiFiClient.write(data); }
        uint_t Write(const u8 *buf, uint_t size) { return gWiFiClient.write(buf, size); }

        nstatus::status_t Connected()
        {
            if (gWiFiClient.connected() == 0)
                return nstatus::Disconnected;
            return nstatus::Connected;
        }
        s32    Available() { return gWiFiClient.available(); }
        s32    Read() { return gWiFiClient.read(); }
        s32    Read(u8 *buf, uint_t size) { return gWiFiClient.read(buf, size); }
        uint_t ReadBytes(char *buffer, uint_t length) { return gWiFiClient.readBytes(buffer, length); }
        uint_t ReadBytes(u8 *buffer, uint_t length) { return gWiFiClient.readBytes(buffer, length); }
        s32    Peek() { return gWiFiClient.peek(); }
        void   Clear() { gWiFiClient.clear(); }
        void   Stop() { gWiFiClient.stop(); }
        void   SetSSE(bool sse) { gWiFiClient.setSSE(sse); }
        bool   IsSSE() { return gWiFiClient.isSSE(); }

        s32  SetSocketOption(s32 option, char *value, uint_t len) { return gWiFiClient.setSocketOption(option, value, len); }
        s32  SetSocketOption(s32 level, s32 option, const void *value, uint_t len) { return gWiFiClient.setSocketOption(level, option, value, len); }
        s32  GetSocketOption(s32 level, s32 option, const void *value, uint_t size) { return gWiFiClient.getSocketOption(level, option, value, size); }
        s32  SetOption(s32 option, s32 *value) { return gWiFiClient.setOption(option, value); }
        s32  GetOption(s32 option, s32 *value) { return gWiFiClient.getOption(option, value); }
        void SetConnectionTimeout(u32 milliseconds) { gWiFiClient.setConnectionTimeout(milliseconds); }
        s32  SetNoDelay(bool nodelay) { return gWiFiClient.setNoDelay(nodelay); }
        bool GetNoDelay() { return gWiFiClient.getNoDelay(); }

        IPAddress_t RemoteIP()
        {
            IPAddress ip = gWiFiClient.remoteIP();
            IPAddress_t rip;
            rip.m_address[0] = ip[0];
            rip.m_address[1] = ip[1];
            rip.m_address[2] = ip[2];
            rip.m_address[3] = ip[3];
            return rip;
        }
        u16         RemotePort() { return gWiFiClient.remotePort(); }
        IPAddress_t LocalIP()
        {
            IPAddress ip = gWiFiClient.localIP();
            IPAddress_t lip;
            lip.m_address[0] = ip[0];
            lip.m_address[1] = ip[1];
            lip.m_address[2] = ip[2];
            lip.m_address[3] = ip[3];
            return lip;
        }
        u16 LocalPort() { return gWiFiClient.localPort(); }

    }  // namespace nclient
}  // namespace ncore

#else

#    include "rdno_wifi/c_client.h"

namespace ncore
{
    namespace nclient
    {
        bool NewClient()
        {
            gNumClients++;
            return gNumClients == 1;
        }

        nstatus::status_t Connect(IPAddress_t ip, u16 port) { return nstatus::Connected; }
        nstatus::status_t Connect(IPAddress_t ip, u16 port, s32 timeout_ms) { return nstatus::Connected; }
        nstatus::status_t Connect(const char *host, u16 port) { return nstatus::Connected; }
        nstatus::status_t Connect(const char *host, u16 port, s32 timeout_ms) { return nstatus::Connected; }
        uint_t            Write(u8 data) { return 1; }
        uint_t            Write(const u8 *buf, uint_t size) { return size; }

        s32               Available() { return 1; }
        s32               Read() { return 1; }
        s32               Read(u8 *buf, uint_t size) { return 1; }
        uint_t            ReadBytes(char *buffer, uint_t length) { return 1; }
        uint_t            ReadBytes(u8 *buffer, uint_t length) { return 1; }
        s32               Peek() { return 0; }
        void              Clear() {}
        void              Stop() {}
        nstatus::status_t Connected() { return nstatus::Connected; }
        void              SetSSE(bool sse) {}
        bool              IsSSE() { return false; }

        s32  SetSocketOption(s32 option, char *value, uint_t len) { return 0; }
        s32  SetSocketOption(s32 level, s32 option, const void *value, uint_t len) { return 0; }
        s32  GetSocketOption(s32 level, s32 option, const void *value, uint_t size) { return 0; }
        s32  SetOption(s32 option, s32 *value) { return 0; }
        s32  GetOption(s32 option, s32 *value) { return 0; }
        void SetConnectionTimeout(u32 milliseconds) {}
        s32  SetNoDelay(bool nodelay) { return 0; }
        bool GetNoDelay() { return false; }

        IPAddress_t RemoteIP() { return IPAddress_t{10, 0, 0, 43}; }
        u16         RemotePort() { return 4242; }
        IPAddress_t LocalIP() { return IPAddress_t{10, 0, 0, 42}; }
        u16         LocalPort() { return 4242; }

    }  // namespace nclient
}  // namespace ncore

#endif
