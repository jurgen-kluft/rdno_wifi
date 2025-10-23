#include "rdno_core/c_str.h"
#include "rdno_wifi/c_tcp.h"

#ifdef TARGET_ARDUINO

#    include "Arduino.h"

#    include "WiFiServer.h"
#    include "WiFiClient.h"

namespace ncore
{
    struct state_tcp_t
    {
        ncore::s16 m_NumClients = 0;
        WiFiClient m_WiFiClient;
    };

    namespace ntcp
    {
#    ifdef TARGET_ESP32
        client_t connect(state_tcp_t* state, IPAddress_t _ip, u16 _port, s32 timeout_ms)
        {
            if (state->m_NumClients == 1)
                return nullptr;
            IPAddress ip(_ip.m_address[0], _ip.m_address[1], _ip.m_address[2], _ip.m_address[3]);
            if (state->m_WiFiClient.connect(ip, _port, timeout_ms) == 0)
                return nullptr;
            state->m_NumClients++;
            return &state->m_WiFiClient;
        }

        bool disconnect(state_tcp_t* state, client_t& client)
        {
            if (client == nullptr)
                return false;

            state->m_WiFiClient.stop();
            state->m_NumClients--;
            client = nullptr;
            return true;
        }

        s32 write(state_tcp_t* state, client_t client, const u8* buf, s32 size)
        {
            if (client == nullptr)
                return 0;
            return (s32)state->m_WiFiClient.write(buf, size);
        }

        nstatus::status_t connected(state_tcp_t* state, client_t client)
        {
            if (client == nullptr)
                return nstatus::Disconnected;
            if (state->m_WiFiClient.connected() == 0)
                return nstatus::Disconnected;
            return nstatus::Connected;
        }

        s32 available(state_tcp_t* state, client_t client)
        {
            if (client == nullptr)
                return 0;
            return state->m_WiFiClient.available();
        }

#    endif

        IPAddress_t remote_IP(state_tcp_t* state, client_t client)
        {
            IPAddress   ip = state->m_WiFiClient.remoteIP();
            IPAddress_t rip;
            rip.m_address[0] = ip[0];
            rip.m_address[1] = ip[1];
            rip.m_address[2] = ip[2];
            rip.m_address[3] = ip[3];
            return rip;
        }

        u16 remote_port(state_tcp_t* state, client_t client) { return state->m_WiFiClient.remotePort(); }

        IPAddress_t local_IP(state_tcp_t* state, client_t client)
        {
            IPAddress   ip = state->m_WiFiClient.localIP();
            IPAddress_t lip;
            lip.m_address[0] = ip[0];
            lip.m_address[1] = ip[1];
            lip.m_address[2] = ip[2];
            lip.m_address[3] = ip[3];
            return lip;
        }

        u16 local_port(state_tcp_t* state, client_t client) { return state->m_WiFiClient.localPort(); }

#    ifdef TARGET_ESP8266

        client_t connect(state_tcp_t* state, IPAddress_t _ip, u16 _port, s32 timeout_ms)
        {
            if (state->m_NumClients == 1)
                return nullptr;
            IPAddress ip(_ip.m_address[0], _ip.m_address[1], _ip.m_address[2], _ip.m_address[3]);
            state->m_WiFiClient.setTimeout(timeout_ms);
            if (state->m_WiFiClient.connect(ip, _port) == 0)
                return nullptr;
            state->m_NumClients++;
            return &state->m_WiFiClient;
        }

        bool disconnect(state_tcp_t* state, client_t& client)
        {
            if (client == nullptr)
                return false;
            state->m_WiFiClient.stop();
            state->m_NumClients--;
            client = nullptr;
            return true;
        }

        s32 write(state_tcp_t* state, client_t client, const u8* buf, s32 size)
        {
            if (client == nullptr)
                return 0;
            return state->m_WiFiClient.write(buf, size);
        }

        nstatus::status_t connected(state_tcp_t* state, client_t client)
        {
            if (client == nullptr)
                return nstatus::Disconnected;
            if (state->m_WiFiClient.connected() == 0)
                return nstatus::Disconnected;
            return nstatus::Connected;
        }

        s32 available(state_tcp_t* state, client_t client)
        {
            if (client == nullptr)
                return 0;
            return state->m_WiFiClient.available();
        }

#    endif

    }  // namespace ntcp
}  // namespace ncore

#else

namespace ncore
{
    namespace ntcp
    {
        client_t          connect(state_tcp_t* state, IPAddress_t ip, u16 port, s32 timeout_ms) { return 1; }
        uint_t            write(state_tcp_t* state, client_t client, const u8* buf, uint_t size) { return size; }
        s32               available(state_tcp_t* state, client_t client) { return 1; }
        void              stop(state_tcp_t* state, client_t client) {}
        nstatus::status_t connected(state_tcp_t* state, client_t client) { return nstatus::Connected; }
        IPAddress_t       remote_IP(state_tcp_t* state, client_t client) { return IPAddress_t{10, 0, 0, 43}; }
        u16               remote_port(state_tcp_t* state, client_t client) { return 4242; }
        IPAddress_t       local_IP(state_tcp_t* state, client_t client) { return IPAddress_t{10, 0, 0, 42}; }
        u16               local_port(state_tcp_t* state, client_t client) { return 4242; }

    }  // namespace ntcp
}  // namespace ncore

#endif
