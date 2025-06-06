#ifdef TARGET_ESP32

#    include "Arduino.h"
#    include "WiFiClient.h"

#    include "rdno_wifi/c_client.h"
#    include "rdno_core/c_allocator.h"

class WiFiClientInstance : public WiFiClient
{
public:
    WiFiClientInstance() : WiFiClient() {}
    virtual ~WiFiClientInstance() {}

    DCORE_CLASS_PLACEMENT_NEW_DELETE
};

namespace ncore
{
    namespace nclient
    {
        s16                 gNumClients = 0;
        WiFiClientInstance *gWiFiClient[4];

        s16 NewClient(alloc_t* allocator)
        {
            gWiFiClient[gNumClients] = allocator->construct<WiFiClientInstance>();
            gNumClients++;
            return gNumClients - 1;
        }

        nstatus::status_t Connect(s16 clientIndex, IPAddress_t _ip, u16 _port)
        {
            IPAddress ip(_ip.A, _ip.B, _ip.C, _ip.D);
            return gWiFiClient[clientIndex]->connect(ip, _port) == 1 ? nstatus::Connected : nstatus::ConnectFailed;
        }
        nstatus::status_t Connect(s16 clientIndex, IPAddress_t _ip, u16 _port, s32 timeout_ms)
        {
            IPAddress ip(_ip.A, _ip.B, _ip.C, _ip.D);
            return gWiFiClient[clientIndex]->connect(ip, _port, timeout_ms) ? nstatus::Connected : nstatus::ConnectFailed;
        }
        nstatus::status_t Connect(s16 clientIndex, const char *host, u16 port) { return gWiFiClient[clientIndex]->connect(host, port) ? nstatus::Connected : nstatus::ConnectFailed; }
        nstatus::status_t Connect(s16 clientIndex, const char *host, u16 port, s32 timeout_ms) { return gWiFiClient[clientIndex]->connect(host, port, timeout_ms) ? nstatus::Connected : nstatus::ConnectFailed; }
        uint_t            Write(s16 clientIndex, u8 data) { return gWiFiClient[clientIndex]->write(data); }
        uint_t            Write(s16 clientIndex, const u8 *buf, uint_t size) { return gWiFiClient[clientIndex]->write(buf, size); }

        s32               Available(s16 clientIndex) { return gWiFiClient[clientIndex]->available(); }
        s32               Read(s16 clientIndex) { return gWiFiClient[clientIndex]->read(); }
        s32               Read(s16 clientIndex, u8 *buf, uint_t size) { return gWiFiClient[clientIndex]->read(buf, size); }
        uint_t            ReadBytes(s16 clientIndex, char *buffer, uint_t length) { return gWiFiClient[clientIndex]->readBytes(buffer, length); }
        uint_t            ReadBytes(s16 clientIndex, u8 *buffer, uint_t length) { return gWiFiClient[clientIndex]->readBytes(buffer, length); }
        s32               Peek(s16 clientIndex) { return gWiFiClient[clientIndex]->peek(); }
        void              Clear(s16 clientIndex) { gWiFiClient[clientIndex]->clear(); }
        void              Stop(s16 clientIndex) { gWiFiClient[clientIndex]->stop(); }
        nstatus::status_t Connected(s16 clientIndex)
        {
            if (gWiFiClient[clientIndex]->connected() == true)
                return nstatus::Connected;
            return nstatus::Disconnected;
        }
        void SetSSE(s16 clientIndex, bool sse) { gWiFiClient[clientIndex]->setSSE(sse); }
        bool IsSSE(s16 clientIndex) { return gWiFiClient[clientIndex]->isSSE(); }

        s32  SetSocketOption(s16 clientIndex, s32 option, char *value, uint_t len) { return gWiFiClient[clientIndex]->setSocketOption(option, value, len); }
        s32  SetSocketOption(s16 clientIndex, s32 level, s32 option, const void *value, uint_t len) { return gWiFiClient[clientIndex]->setSocketOption(level, option, value, len); }
        s32  GetSocketOption(s16 clientIndex, s32 level, s32 option, const void *value, uint_t size) { return gWiFiClient[clientIndex]->getSocketOption(level, option, value, size); }
        s32  SetOption(s16 clientIndex, s32 option, s32 *value) { return gWiFiClient[clientIndex]->setOption(option, value); }
        s32  GetOption(s16 clientIndex, s32 option, s32 *value) { return gWiFiClient[clientIndex]->getOption(option, value); }
        void SetConnectionTimeout(s16 clientIndex, u32 milliseconds) { gWiFiClient[clientIndex]->setConnectionTimeout(milliseconds); }
        s32  SetNoDelay(s16 clientIndex, bool nodelay) { return gWiFiClient[clientIndex]->setNoDelay(nodelay); }
        bool GetNoDelay(s16 clientIndex) { return gWiFiClient[clientIndex]->getNoDelay(); }

        IPAddress_t RemoteIP(s16 clientIndex)
        {
            IPAddress ip = gWiFiClient[clientIndex]->remoteIP();
            return IPAddress_t(ip[0], ip[1], ip[2], ip[3]);
        }
        IPAddress_t RemoteIP(s16 clientIndex, s32 fd)
        {
            IPAddress ip = gWiFiClient[clientIndex]->remoteIP(fd);
            return IPAddress_t(ip[0], ip[1], ip[2], ip[3]);
        }
        u16         RemotePort(s16 clientIndex) { return gWiFiClient[clientIndex]->remotePort(); }
        u16         RemotePort(s16 clientIndex, s32 fd) { return gWiFiClient[clientIndex]->remotePort(fd); }
        IPAddress_t LocalIP(s16 clientIndex)
        {
            IPAddress ip = gWiFiClient[clientIndex]->localIP();
            return IPAddress_t(ip[0], ip[1], ip[2], ip[3]);
        }
        IPAddress_t LocalIP(s16 clientIndex, s32 fd)
        {
            IPAddress ip = gWiFiClient[clientIndex]->localIP(fd);
            return IPAddress_t(ip[0], ip[1], ip[2], ip[3]);
        }
        u16 LocalPort(s16 clientIndex) { return gWiFiClient[clientIndex]->localPort(); }
        u16 LocalPort(s16 clientIndex, s32 fd) { return gWiFiClient[clientIndex]->localPort(fd); }

    }  // namespace nclient
}  // namespace ncore

#else

#    include "rdno_wifi/c_client.h"
#    include "rdno_core/c_allocator.h"

namespace ncore
{
    namespace nclient
    {
        struct WifiClient
        {
            WifiClient()
                : fd(-1)
                , status(nstatus::Disconnected)
                , sse(false)
            {
            }

            DCORE_CLASS_PLACEMENT_NEW_DELETE

            s16  fd;
            s16  status;
            bool sse;
        };

        s16         gNumClients = 0;
        WifiClient *sWifiClients[4];

        s16 NewClient(alloc_t* allocator)
        {
            sWifiClients[gNumClients] = allocator->construct<WifiClient>();
            gNumClients++;
            return gNumClients - 1;
        }

        nstatus::status_t Connect(s16 clientIndex, IPAddress_t ip, u16 port)
        {
            sWifiClients[clientIndex]->status = nstatus::Connected;
            return sWifiClients[clientIndex]->status;
        }
        nstatus::status_t Connect(s16 clientIndex, IPAddress_t ip, u16 port, s32 timeout_ms)
        {
            sWifiClients[clientIndex]->status = nstatus::Connected;
            return sWifiClients[clientIndex]->status;
        }
        nstatus::status_t Connect(s16 clientIndex, const char *host, u16 port)
        {
            sWifiClients[clientIndex]->status = nstatus::Connected;
            return sWifiClients[clientIndex]->status;
        }
        nstatus::status_t Connect(s16 clientIndex, const char *host, u16 port, s32 timeout_ms)
        {
            sWifiClients[clientIndex]->status = nstatus::Connected;
            return sWifiClients[clientIndex]->status;
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

    }  // namespace nclient
}  // namespace ncore

#endif
