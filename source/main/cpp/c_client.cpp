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

        void InitMaxTcpClients(alloc_t *allocator, u16 max_clients)
        {
            sWifiClients = g_allocate_array_and_clear<WiFiClient *>(allocator, max_clients);
            sMaxClients  = max_clients;
            sNumClients  = 0;
        }

        s16 NewTcpClient(alloc_t *allocator)
        {
            if (sNumClients >= sMaxClients)
            {
                return -1;
            }

            s16 clientIndex           = sNumClients++;
            sWifiClients[clientIndex] = g_allocate<WiFiClient>(allocator);
            return clientIndex;
        }

        s32 connect(s16 clientIndex, IPAddress_t _ip, u16 _port)
        {
            IPAddress ip(_ip.A, _ip.B, _ip.C, _ip.D);
            return sWifiClients[clientIndex]->connect(ip, _port);
        }
        s32 connect(s16 clientIndex, IPAddress_t _ip, u16 _port, s32 timeout_ms)
        {
            IPAddress ip(_ip.A, _ip.B, _ip.C, _ip.D);
            return sWifiClients[clientIndex]->connect(ip, _port, timeout_ms);
        }
        s32    connect(s16 clientIndex, const char *host, u16 port) { return sWifiClients[clientIndex]->connect(host, port); }
        s32    connect(s16 clientIndex, const char *host, u16 port, s32 timeout_ms) { return sWifiClients[clientIndex]->connect(host, port, timeout_ms); }
        uint_t write(s16 clientIndex, u8 data) { return sWifiClients[clientIndex]->write(data); }
        uint_t write(s16 clientIndex, const u8 *buf, uint_t size) { return sWifiClients[clientIndex]->write(buf, size); }

        s32    available(s16 clientIndex) { return sWifiClients[clientIndex]->available(); }
        s32    read(s16 clientIndex) { return sWifiClients[clientIndex]->read(); }
        s32    read(s16 clientIndex, u8 *buf, uint_t size) { return sWifiClients[clientIndex]->read(buf, size); }
        uint_t readBytes(s16 clientIndex, char *buffer, uint_t length) { return sWifiClients[clientIndex]->readBytes(buffer, length); }
        uint_t readBytes(s16 clientIndex, u8 *buffer, uint_t length) { return sWifiClients[clientIndex]->readBytes(buffer, length); }
        s32    peek(s16 clientIndex) { return sWifiClients[clientIndex]->peek(); }
        void   clear(s16 clientIndex) { sWifiClients[clientIndex]->clear(); }
        void   stop(s16 clientIndex) { sWifiClients[clientIndex]->stop(); }
        u8     connected(s16 clientIndex) { return sWifiClients[clientIndex]->connected(); }
        void   setSSE(s16 clientIndex, bool sse) { sWifiClients[clientIndex]->setSSE(sse); }
        bool   isSSE(s16 clientIndex) { return sWifiClients[clientIndex]->isSSE(); }

        s32  setSocketOption(s16 clientIndex, s32 option, char *value, uint_t len) { return sWifiClients[clientIndex]->setSocketOption(option, value, len); }
        s32  setSocketOption(s16 clientIndex, s32 level, s32 option, const void *value, uint_t len) { return sWifiClients[clientIndex]->setSocketOption(level, option, value, len); }
        s32  getSocketOption(s16 clientIndex, s32 level, s32 option, const void *value, uint_t size) { return sWifiClients[clientIndex]->getSocketOption(level, option, value, size); }
        s32  setOption(s16 clientIndex, s32 option, s32 *value) { return sWifiClients[clientIndex]->setOption(option, value); }
        s32  getOption(s16 clientIndex, s32 option, s32 *value) { return sWifiClients[clientIndex]->getOption(option, value); }
        void setConnectionTimeout(s16 clientIndex, u32 milliseconds) { sWifiClients[clientIndex]->setConnectionTimeout(milliseconds); }
        s32  setNoDelay(s16 clientIndex, bool nodelay) { return sWifiClients[clientIndex]->setNoDelay(nodelay); }
        bool getNoDelay(s16 clientIndex) { return sWifiClients[clientIndex]->getNoDelay(); }

        IPAddress_t remoteIP(s16 clientIndex)
        {
            IPAddress ip = sWifiClients[clientIndex]->remoteIP();
            return IPAddress_t(ip[0], ip[1], ip[2], ip[3]);
        }
        IPAddress_t remoteIP(s16 clientIndex, s32 fd)
        {
            IPAddress ip = sWifiClients[clientIndex]->remoteIP(fd);
            return IPAddress_t(ip[0], ip[1], ip[2], ip[3]);
        }
        u16         remotePort(s16 clientIndex) { return sWifiClients[clientIndex]->remotePort(); }
        u16         remotePort(s16 clientIndex, s32 fd) { return sWifiClients[clientIndex]->remotePort(fd); }
        IPAddress_t localIP(s16 clientIndex)
        {
            IPAddress ip = sWifiClients[clientIndex]->localIP();
            return IPAddress_t(ip[0], ip[1], ip[2], ip[3]);
        }
        IPAddress_t localIP(s16 clientIndex, s32 fd)
        {
            IPAddress ip = sWifiClients[clientIndex]->localIP(fd);
            return IPAddress_t(ip[0], ip[1], ip[2], ip[3]);
        }
        u16 localPort(s16 clientIndex) { return sWifiClients[clientIndex]->localPort(); }
        u16 localPort(s16 clientIndex, s32 fd) { return sWifiClients[clientIndex]->localPort(fd); }

    }  // namespace nwifi
}  // namespace ncore

#else

#    include "rdno_wifi/c_client.h"

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

        void InitMaxTcpClients(alloc_t *allocator, u16 max_clients)
        {
            sWifiClients = g_allocate_array_and_clear<WifiClient *>(allocator, max_clients);
            sNumClients  = 0;
            sMaxClients  = max_clients;
        }

        s16 NewTcpClient(alloc_t *allocator)
        {
            if (sNumClients >= sMaxClients)
            {
                return -1;
            }

            s16 clientIndex           = sNumClients++;
            sWifiClients[clientIndex] = g_allocate<WifiClient>(allocator);
            return clientIndex;
        }

        s32 connect(s16 clientIndex, IPAddress_t ip, u16 port)
        {
            sWifiClients[clientIndex]->status = nstatus::Connected;
            return nstatus::Connected;
        }
        s32 connect(s16 clientIndex, IPAddress_t ip, u16 port, s32 timeout_ms)
        {
            sWifiClients[clientIndex]->status = nstatus::Connected;
            return nstatus::Connected;
        }
        s32 connect(s16 clientIndex, const char *host, u16 port)
        {
            sWifiClients[clientIndex]->status = nstatus::Connected;
            return nstatus::Connected;
        }
        s32 connect(s16 clientIndex, const char *host, u16 port, s32 timeout_ms)
        {
            sWifiClients[clientIndex]->status = nstatus::Connected;
            return nstatus::Connected;
        }
        uint_t write(s16 clientIndex, u8 data) { return 1; }
        uint_t write(s16 clientIndex, const u8 *buf, uint_t size) { return size; }

        s32    available(s16 clientIndex) { return 1; }
        s32    read(s16 clientIndex) { return 1; }
        s32    read(s16 clientIndex, u8 *buf, uint_t size) { return 1; }
        uint_t readBytes(s16 clientIndex, char *buffer, uint_t length) { return 1; }
        uint_t readBytes(s16 clientIndex, u8 *buffer, uint_t length) { return 1; }
        s32    peek(s16 clientIndex) { return 0; }
        void   clear(s16 clientIndex) {}
        void   stop(s16 clientIndex) {}
        u8     connected(s16 clientIndex) { return sWifiClients[clientIndex]->status == nstatus::Connected; }
        void   setSSE(s16 clientIndex, bool sse) { sWifiClients[clientIndex]->sse = sse; }
        bool   isSSE(s16 clientIndex) { return sWifiClients[clientIndex]->sse; }

        s32  setSocketOption(s16 clientIndex, s32 option, char *value, uint_t len) { return 0; }
        s32  setSocketOption(s16 clientIndex, s32 level, s32 option, const void *value, uint_t len) { return 0; }
        s32  getSocketOption(s16 clientIndex, s32 level, s32 option, const void *value, uint_t size) { return 0; }
        s32  setOption(s16 clientIndex, s32 option, s32 *value) { return 0; }
        s32  getOption(s16 clientIndex, s32 option, s32 *value) { return 0; }
        void setConnectionTimeout(s16 clientIndex, u32 milliseconds) {}
        s32  setNoDelay(s16 clientIndex, bool nodelay) { return 0; }
        bool getNoDelay(s16 clientIndex) { return false; }

        IPAddress_t remoteIP(s16 clientIndex) { return IPAddress_t(); }
        IPAddress_t remoteIP(s16 clientIndex, s32 fd) { return IPAddress_t(); }
        u16         remotePort(s16 clientIndex) { return 0; }
        u16         remotePort(s16 clientIndex, s32 fd) { return 0; }
        IPAddress_t localIP(s16 clientIndex) { return IPAddress_t(); }
        IPAddress_t localIP(s16 clientIndex, s32 fd) { return IPAddress_t(); }
        u16         localPort(s16 clientIndex) { return 0; }
        u16         localPort(s16 clientIndex, s32 fd) { return 0; }

    }  // namespace nwifi
}  // namespace ncore

#endif
