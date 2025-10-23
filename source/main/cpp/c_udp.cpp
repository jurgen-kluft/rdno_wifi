#include "rdno_core/c_str.h"
#include "rdno_wifi/c_udp.h"

#ifdef TARGET_ARDUINO

#    include "Arduino.h"
#    include "WiFiUdp.h"
#    include "NetworkUdp.h"
#    include <lwip/sockets.h>
#    include <lwip/netdb.h>
#    include <errno.h>

namespace ncore
{
    namespace nudp
    {
#    ifdef TARGET_ESP32
        bool open(sock_t &sock, u16 port)
        {
            if (sock.m_port != port)
            {
                close(sock);
            }

            if (sock.m_fd != -1)
                return true;

            IPAddress address(IPv4);
            sock.m_port = port;

            if ((sock.m_fd = socket(AF_INET, SOCK_DGRAM, 0)) == -1)
            {
                log_e("could not create socket: %d", errno);
                return false;
            }

            int yes = 1;
            if (setsockopt(sock.m_fd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(yes)) < 0)
            {
                log_e("could not set socket option: %d", errno);
                close(sock);
                return false;
            }

            struct sockaddr_storage serveraddr = {};
            size_t                  sock_size  = 0;
            {
                struct sockaddr_in *tmpaddr = (struct sockaddr_in *)&serveraddr;
                memset((char *)tmpaddr, 0, sizeof(struct sockaddr_in));
                tmpaddr->sin_family      = AF_INET;
                tmpaddr->sin_port        = htons(sock.m_port);
                tmpaddr->sin_addr.s_addr = (in_addr_t)address;
                sock_size                = sizeof(sockaddr_in);
            }
            if (bind(sock.m_fd, (sockaddr *)&serveraddr, sock_size) == -1)
            {
                log_e("could not bind socket: %d", errno);
                close(sock);
                return false;
            }
            fcntl(sock.m_fd, F_SETFL, O_NONBLOCK);
            return true;
        }

        void close(sock_t &sock)
        {
            if (sock.m_fd == -1)
                return;
            ::close(sock.m_fd);
            sock.m_fd   = -1;
            sock.m_port = 0;
        }

        s32 receive(sock_t &sock, byte *rxdata, s32 max_rxdatasize, IPAddress_t &remote_ip, u16 &remote_port)
        {
            if (sock.m_fd == -1)
            {
                return 0;
            }

            struct sockaddr_storage si_other_storage;  // enough storage for v4 and v6
            socklen_t               slen = sizeof(sockaddr_storage);
            int                     len  = 0;
            if (ioctl(sock.m_fd, FIONREAD, &len) == -1)
            {
                log_e("could not check for data in buffer length: %d", errno);
                return 0;
            }
            if (!len)
            {
                return 0;
            }
            if ((len = recvfrom(sock.m_fd, rxdata, 1460, MSG_DONTWAIT, (struct sockaddr *)&si_other_storage, (socklen_t *)&slen)) == -1)
            {
                if (errno == EWOULDBLOCK)
                {
                    return 0;
                }
                log_e("could not receive data: %d", errno);
                return 0;
            }
            if (si_other_storage.ss_family == AF_INET)
            {
                struct sockaddr_in &si_other = (sockaddr_in &)si_other_storage;
                remote_ip.from(si_other.sin_addr.s_addr);
                remote_port = ntohs(si_other.sin_port);
            }
            else
            {
                remote_ip.from((u32)ip_addr_any.u_addr.ip4.addr);
                remote_port = 0;
            }
            return len;
        }

        s32 send_to(sock_t &sock, byte const *data, s32 data_size, const IPAddress_t &to_address, u16 to_port)
        {
            byte const *tx_buffer     = data;
            u32 const   tx_buffer_len = data_size;

            IPAddress remote_ip(to_address.m_address[0], to_address.m_address[1], to_address.m_address[2], to_address.m_address[3]);
            ip_addr_t addr;
            remote_ip.to_ip_addr_t(&addr);

            if (remote_ip.type() == IPv4)
            {
                struct sockaddr_in recipient;
                recipient.sin_addr.s_addr = (uint32_t)remote_ip;
                recipient.sin_family      = AF_INET;
                recipient.sin_port        = htons(to_port);
                int sent                  = sendto(sock.m_fd, tx_buffer, tx_buffer_len, 0, (struct sockaddr *)&recipient, sizeof(recipient));
                if (sent < 0)
                {
                    log_e("could not send data: %d", errno);
                    return 0;
                }
            }
            return data_size;
        }
#    endif

#    ifdef TARGET_ESP8266
        bool open(sock_t &sock, u16 port)
        {
            if (sock.m_port != port)
            {
                close();
            }

            if (sock.m_fd != -1)
                return true;

            IPAddress address();
            sock.m_port = port;

            if ((sock.m_fd = socket(AF_INET, SOCK_DGRAM, 0)) == -1)
            {
                log_e("could not create socket: %d", errno);
                return false;
            }

            int yes = 1;
            if (setsockopt(sock.m_fd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(yes)) < 0)
            {
                log_e("could not set socket option: %d", errno);
                stop();
                return false;
            }

            struct sockaddr_storage serveraddr = {};
            size_t                  sock_size  = 0;
            {
                struct sockaddr_in *tmpaddr = (struct sockaddr_in *)&serveraddr;
                memset((char *)tmpaddr, 0, sizeof(struct sockaddr_in));
                tmpaddr->sin_family      = AF_INET;
                tmpaddr->sin_port        = htons(sock.m_port);
                tmpaddr->sin_addr.s_addr = (in_addr_t)address;
                sock_size                = sizeof(sockaddr_in);
            }
            if (bind(sock.m_fd, (sockaddr *)&serveraddr, sock_size) == -1)
            {
                log_e("could not bind socket: %d", errno);
                stop();
                return false;
            }
            fcntl(sock.m_fd, F_SETFL, O_NONBLOCK);
            return true;
        }

        void close(sock_t &sock)
        {
            if (sock.m_fd == -1)
                return;
            close(sock.m_fd);
            sock.m_fd   = -1;
            sock.m_port = 0;
        }

        s32 receive(sock_t &sock, byte *rxdata, s32 max_rxdatasize, IPAddress_t &remote_ip, u16 &remote_port)
        {
            if (sock.m_fd == -1)
            {
                return 0;
            }

            struct sockaddr_storage si_other_storage;  // enough storage for v4 and v6
            socklen_t               slen = sizeof(sockaddr_storage);
            int                     len  = 0;
            if (ioctl(sock.m_fd, FIONREAD, &len) == -1)
            {
                log_e("could not check for data in buffer length: %d", errno);
                return 0;
            }
            if (!len)
            {
                return 0;
            }
            if ((len = recvfrom(sock.m_fd, rxdata, 1460, MSG_DONTWAIT, (struct sockaddr *)&si_other_storage, (socklen_t *)&slen)) == -1)
            {
                if (errno == EWOULDBLOCK)
                {
                    return 0;
                }
                log_e("could not receive data: %d", errno);
                return 0;
            }
            if (si_other_storage.ss_family == AF_INET)
            {
                struct sockaddr_in &si_other = (sockaddr_in &)si_other_storage;
                remote_ip                    = IPAddress(si_other.sin_addr.s_addr);
                remote_port                  = ntohs(si_other.sin_port);
            }
            else
            {
                remote_ip   = ip_addr_any.addr;
                remote_port = 0;
            }
            return len;
        }

        s32 send_to(sock_t &sock, byte const *data, s32 data_size, const IPAddress_t &to_address, u16 to_port)
        {
            byte const *tx_buffer     = data;
            u32 const   tx_buffer_len = data_size;

            IPAddress remote_ip(to_address.m_address[0], to_address.m_address[1], to_address.m_address[2], to_address.m_address[3]);
            ip_addr_t addr;
            remote_ip.to_ip_addr_t(&addr);

            if (remote_ip.type() == IPv4)
            {
                struct sockaddr_in recipient;
                recipient.sin_addr.s_addr = (uint32_t)remote_ip;
                recipient.sin_family      = AF_INET;
                recipient.sin_port        = htons(to_port);
                int sent                  = sendto(sock.m_fd, tx_buffer, tx_buffer_len, 0, (struct sockaddr *)&recipient, sizeof(recipient));
                if (sent < 0)
                {
                    log_e("could not send data: %d", errno);
                    return 0;
                }
            }
            return data_size;
        }
#    endif

    }  // namespace nudp
}  // namespace ncore

#else

namespace ncore
{
    namespace nudp
    {
        bool open(sock_t& sock, u16 port) { return false; }
        void close(sock_t& sock) {}

        s32 receive(sock_t& sock, byte* data, s32 max_data_size, IPAddress_t& from_address) { return 0; }
        s32 send_to(sock_t& sock, byte const* data, s32 data_size, IPAddress_t to_address, u16 to_port) { return 0; }
    }  // namespace nudp

}  // namespace ncore

#endif
