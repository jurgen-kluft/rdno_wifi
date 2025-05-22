#ifdef TARGET_ESP32

#    include "Arduino.h"
#    include "WiFiClient.h"

#    include "rdno_wifi/c_client.h"
#    include "rdno_core/c_allocator.h"

namespace ncore
{
    namespace nwifi
    {
        static WiFiClient **sWifiClients = nullptr;
        static s16          sMaxClients  = 0;
        static s16          sNumClients  = 0;

        void InitMaxClients(alloc_t *allocator, u16 max_clients)
        {
            sWifiClients = g_allocate_array_and_clear<WiFiClient *>(allocator, max_clients);
            sMaxClients  = max_clients;
            sNumClients  = 0;
        }

        s16 NewClient(alloc_t *allocator)
        {
            if (sNumClients >= sMaxClients)
            {
                if (sMaxClients == 0)
                {
                    InitMaxClients(allocator, 1);
                    sMaxClients = 1;
                }
                else
                {
                    return -1;
                }
            }

            s16 clientIndex           = sNumClients++;
            sWifiClients[clientIndex] = g_allocate<WiFiClient>(allocator);
            return clientIndex;
        }

        s32 Connect(s16 clientIndex, IPAddress_t _ip, u16 _port)
        {
            IPAddress ip(_ip.A, _ip.B, _ip.C, _ip.D);
            return sWifiClients[clientIndex]->connect(ip, _port);
        }
        s32 Connect(s16 clientIndex, IPAddress_t _ip, u16 _port, s32 timeout_ms)
        {
            IPAddress ip(_ip.A, _ip.B, _ip.C, _ip.D);
            return sWifiClients[clientIndex]->connect(ip, _port, timeout_ms);
        }
        s32    Connect(s16 clientIndex, const char *host, u16 port) { return sWifiClients[clientIndex]->connect(host, port); }
        s32    Connect(s16 clientIndex, const char *host, u16 port, s32 timeout_ms) { return sWifiClients[clientIndex]->connect(host, port, timeout_ms); }
        uint_t Write(s16 clientIndex, u8 data) { return sWifiClients[clientIndex]->write(data); }
        uint_t Write(s16 clientIndex, const u8 *buf, uint_t size) { return sWifiClients[clientIndex]->write(buf, size); }

        s32    Available(s16 clientIndex) { return sWifiClients[clientIndex]->available(); }
        s32    Read(s16 clientIndex) { return sWifiClients[clientIndex]->read(); }
        s32    Read(s16 clientIndex, u8 *buf, uint_t size) { return sWifiClients[clientIndex]->read(buf, size); }
        uint_t ReadBytes(s16 clientIndex, char *buffer, uint_t length) { return sWifiClients[clientIndex]->readBytes(buffer, length); }
        uint_t ReadBytes(s16 clientIndex, u8 *buffer, uint_t length) { return sWifiClients[clientIndex]->readBytes(buffer, length); }
        s32    Peek(s16 clientIndex) { return sWifiClients[clientIndex]->peek(); }
        void   Clear(s16 clientIndex) { sWifiClients[clientIndex]->clear(); }
        void   Stop(s16 clientIndex) { sWifiClients[clientIndex]->stop(); }
        u8     Connected(s16 clientIndex) { return sWifiClients[clientIndex]->connected(); }
        void   SetSSE(s16 clientIndex, bool sse) { sWifiClients[clientIndex]->setSSE(sse); }
        bool   IsSSE(s16 clientIndex) { return sWifiClients[clientIndex]->isSSE(); }

        s32  SetSocketOption(s16 clientIndex, s32 option, char *value, uint_t len) { return sWifiClients[clientIndex]->setSocketOption(option, value, len); }
        s32  SetSocketOption(s16 clientIndex, s32 level, s32 option, const void *value, uint_t len) { return sWifiClients[clientIndex]->setSocketOption(level, option, value, len); }
        s32  GetSocketOption(s16 clientIndex, s32 level, s32 option, const void *value, uint_t size) { return sWifiClients[clientIndex]->getSocketOption(level, option, value, size); }
        s32  SetOption(s16 clientIndex, s32 option, s32 *value) { return sWifiClients[clientIndex]->setOption(option, value); }
        s32  GetOption(s16 clientIndex, s32 option, s32 *value) { return sWifiClients[clientIndex]->getOption(option, value); }
        void SetConnectionTimeout(s16 clientIndex, u32 milliseconds) { sWifiClients[clientIndex]->setConnectionTimeout(milliseconds); }
        s32  SetNoDelay(s16 clientIndex, bool nodelay) { return sWifiClients[clientIndex]->setNoDelay(nodelay); }
        bool GetNoDelay(s16 clientIndex) { return sWifiClients[clientIndex]->getNoDelay(); }

        IPAddress_t RemoteIP(s16 clientIndex)
        {
            IPAddress ip = sWifiClients[clientIndex]->remoteIP();
            return IPAddress_t(ip[0], ip[1], ip[2], ip[3]);
        }
        IPAddress_t RemoteIP(s16 clientIndex, s32 fd)
        {
            IPAddress ip = sWifiClients[clientIndex]->remoteIP(fd);
            return IPAddress_t(ip[0], ip[1], ip[2], ip[3]);
        }
        u16         RemotePort(s16 clientIndex) { return sWifiClients[clientIndex]->remotePort(); }
        u16         RemotePort(s16 clientIndex, s32 fd) { return sWifiClients[clientIndex]->remotePort(fd); }
        IPAddress_t LocalIP(s16 clientIndex)
        {
            IPAddress ip = sWifiClients[clientIndex]->localIP();
            return IPAddress_t(ip[0], ip[1], ip[2], ip[3]);
        }
        IPAddress_t LocalIP(s16 clientIndex, s32 fd)
        {
            IPAddress ip = sWifiClients[clientIndex]->localIP(fd);
            return IPAddress_t(ip[0], ip[1], ip[2], ip[3]);
        }
        u16 LocalPort(s16 clientIndex) { return sWifiClients[clientIndex]->localPort(); }
        u16 LocalPort(s16 clientIndex, s32 fd) { return sWifiClients[clientIndex]->localPort(fd); }

    }  // namespace nwifi
}  // namespace ncore

#else

#    include "rdno_wifi/c_client.h"
#    include "rdno_core/c_allocator.h"

namespace ncore
{
    namespace nwifi
    {
        struct WifiClient
        {
            WifiClient()
                : fd(-1)
                , status(nstatus::Disconnected)
                , sse(false)
            {
            }

            s16  fd;
            s16  status;
            bool sse;
        };

        static WifiClient **sWifiClients = nullptr;
        static s16          sNumClients  = 0;
        static s16          sMaxClients  = 0;

        void InitMaxClients(alloc_t *allocator, u16 max_clients)
        {
            sWifiClients = g_allocate_array_and_clear<WifiClient *>(allocator, max_clients);
            sNumClients  = 0;
            sMaxClients  = max_clients;
        }

        s16 NewClient(alloc_t *allocator)
        {
            if (sNumClients >= sMaxClients)
            {
                if (sMaxClients == 0)
                {
                    InitMaxClients(allocator, 1);
                    sMaxClients = 1;
                }
                else
                {
                    return -1;
                }
            }

            s16 clientIndex           = sNumClients++;
            sWifiClients[clientIndex] = g_allocate<WifiClient>(allocator);
            return clientIndex;
        }

        s32 Connect(s16 clientIndex, IPAddress_t ip, u16 port)
        {
            sWifiClients[clientIndex]->status = nstatus::Connected;
            return nstatus::Connected;
        }
        s32 Connect(s16 clientIndex, IPAddress_t ip, u16 port, s32 timeout_ms)
        {
            sWifiClients[clientIndex]->status = nstatus::Connected;
            return nstatus::Connected;
        }
        s32 Connect(s16 clientIndex, const char *host, u16 port)
        {
            sWifiClients[clientIndex]->status = nstatus::Connected;
            return nstatus::Connected;
        }
        s32 Connect(s16 clientIndex, const char *host, u16 port, s32 timeout_ms)
        {
            sWifiClients[clientIndex]->status = nstatus::Connected;
            return nstatus::Connected;
        }
        uint_t Write(s16 clientIndex, u8 data) { return 1; }
        uint_t Write(s16 clientIndex, const u8 *buf, uint_t size) { return size; }

        s32    Available(s16 clientIndex) { return 1; }
        s32    Read(s16 clientIndex) { return 1; }
        s32    Read(s16 clientIndex, u8 *buf, uint_t size) { return 1; }
        uint_t ReadBytes(s16 clientIndex, char *buffer, uint_t length) { return 1; }
        uint_t ReadBytes(s16 clientIndex, u8 *buffer, uint_t length) { return 1; }
        s32    Peek(s16 clientIndex) { return 0; }
        void   Clear(s16 clientIndex) {}
        void   Stop(s16 clientIndex) {}
        u8     Connected(s16 clientIndex) { return sWifiClients[clientIndex]->status == nstatus::Connected; }
        void   SetSSE(s16 clientIndex, bool sse) { sWifiClients[clientIndex]->sse = sse; }
        bool   IsSSE(s16 clientIndex) { return sWifiClients[clientIndex]->sse; }

        s32  SetSocketOption(s16 clientIndex, s32 option, char *value, uint_t len) { return 0; }
        s32  SetSocketOption(s16 clientIndex, s32 level, s32 option, const void *value, uint_t len) { return 0; }
        s32  GetSocketOption(s16 clientIndex, s32 level, s32 option, const void *value, uint_t size) { return 0; }
        s32  SetOption(s16 clientIndex, s32 option, s32 *value) { return 0; }
        s32  GetOption(s16 clientIndex, s32 option, s32 *value) { return 0; }
        void SetConnectionTimeout(s16 clientIndex, u32 milliseconds) {}
        s32  SetNoDelay(s16 clientIndex, bool nodelay) { return 0; }
        bool GetNoDelay(s16 clientIndex) { return false; }

        IPAddress_t RemoteIP(s16 clientIndex) { return IPAddress_t(); }
        IPAddress_t RemoteIP(s16 clientIndex, s32 fd) { return IPAddress_t(); }
        u16         RemotePort(s16 clientIndex) { return 0; }
        u16         RemotePort(s16 clientIndex, s32 fd) { return 0; }
        IPAddress_t LocalIP(s16 clientIndex) { return IPAddress_t(); }
        IPAddress_t LocalIP(s16 clientIndex, s32 fd) { return IPAddress_t(); }
        u16         LocalPort(s16 clientIndex) { return 0; }
        u16         LocalPort(s16 clientIndex, s32 fd) { return 0; }

    }  // namespace nwifi
}  // namespace ncore

#endif
