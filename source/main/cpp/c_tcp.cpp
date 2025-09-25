#include "rdno_core/c_str.h"
#include "rdno_wifi/c_tcp.h"

#ifdef TARGET_ESP32

#    include "Arduino.h"

#    include "WiFiServer.h"
#    include "WiFiClient.h"

namespace ncore
{
    namespace ntcp
    {
        WiFiServer gTcpServer;

        void server_start(u16 port)
        {
            gTcpServer = WiFiServer(port);
            gTcpServer.begin();
        }

        WiFiClient gTcpClient;

        bool     client_is_connected(client_t client)
        {
            if (client == nullptr)
                return false;

            WiFiClient* tcpClient = static_cast<WiFiClient*>(client);
            return tcpClient->connected();
        }

        client_t server_handle_client()
        {
            if (gTcpServer)
            {
                if (gTcpServer.hasClient())
                {
                    gTcpClient = gTcpServer.accept();
                    return &gTcpClient;
                }
            }
            return nullptr;
        }

        bool client_recv_msg(client_t client, str_t& msg)
        {
            if (!gTcpServer)
                return false;
            if (client == nullptr)
                return false;

            WiFiClient* tcpClient = static_cast<WiFiClient*>(client);
            if (tcpClient->available() > 0)
            {
                while (tcpClient->connected() && tcpClient->available() > 0 && msg.m_end < msg.m_eos)
                {
                    const char c             = tcpClient->read();
                    msg.m_ascii[msg.m_end++] = c;
                }
                msg.m_ascii[msg.m_end] = 0;
                return true;
            }
            return false;
        }

        void server_stop()
        {
            if (gTcpClient)
                gTcpClient.stop();

            if (gTcpServer)
                gTcpServer.stop();
        }

    }  // namespace ntcp
}  // namespace ncore

#else

namespace ncore
{
    namespace ntcp
    {

    }  // namespace ntcp
}  // namespace ncore

#endif
