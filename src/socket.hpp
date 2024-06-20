//! Network socket support.

#ifndef SOCKET_HPP
#define SOCKET_HPP

#include <cerrno>
#include <cstdint>
#include <mutex>
#include <optional>

#include "macros.hpp"

#if defined(OS_WINDOWS)
#   include <Winsock2.h>
#   include <io.h>
#   pragma comment(lib, "ws2_32.lib")
#   define SOCKET_WINSOCK
#else
#   include <unistd.h>
#   include <sys/socket.h>
#   include <netinet/in.h>
#   include <netinet/ip.h>
#endif

#if defined(SOCKET_WINSOCK)
using socklen_t = int;
using ssize_t = SSIZE_T;
#endif

/// Socket Address.
template <int AF>
class SockAddr;

/// IPv4 Socket Address.
template <>
class SockAddr<AF_INET> {
public:
    static constexpr size_t Size = sizeof(struct sockaddr_in);
    static constexpr int Family = AF_INET;

    SockAddr(uint32_t addr, uint16_t port) {
        m_sockaddr.sin_family = Family;
        m_sockaddr.sin_addr.s_addr = htonl(addr);
        m_sockaddr.sin_port = htons(port);
    }

    SockAddr(struct in_addr addr, uint16_t port) {
        m_sockaddr.sin_family = Family;
        m_sockaddr.sin_addr = addr;
        m_sockaddr.sin_port = htons(port);
    }

    operator struct sockaddr_in&() { return m_sockaddr; }
    operator const struct sockaddr_in&() const { return m_sockaddr; }
    operator struct sockaddr&() { return reinterpret_cast<struct sockaddr&>(m_sockaddr); }
    operator const struct sockaddr&() const { return reinterpret_cast<const struct sockaddr&>(m_sockaddr); }

private:
    struct sockaddr_in m_sockaddr{0};
};

/// Network Socket.
template <typename SA = SockAddr<AF_INET>>
class Socket final {
public:
    using SockAddr = SA;

    static Socket udp() {
        init();
        return Socket(socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP));
    }

    Socket(int&& fd = -1) : m_fd(std::move(fd)) {
        // We take ownership of the FD
        fd = -1;
    }

    ~Socket() {
        close();
    }

    Socket(const Socket&) = delete;
    Socket& operator=(const Socket&) = delete;
    Socket(Socket&& other) : m_fd{-1} { std::swap(m_fd, other.m_fd); }
    Socket& operator=(Socket&& other) { close(); std::swap(m_fd, other.m_fd); return *this; };

    operator bool() { return valid(); }
    bool valid() const { return m_fd != -1; }
    int fd() const { return m_fd; }

    void close() {
        if (valid()) {
#if defined(SOCKET_WINSOCK)
            _close(m_fd);
#else
            close(m_fd);
#endif
        }
        m_fd = -1;
    }

    int set_reuseaddr(bool enabled) {
        int value = enabled ? 1 : 0;
        return Socket::setsockopt(m_fd, SOL_SOCKET, SO_REUSEADDR, &value, sizeof(value));
    }

    int bind(const SockAddr& sockaddr) {
        return bind(&static_cast<const struct sockaddr&>(sockaddr), SockAddr::Size);
    }

    int bind(const struct sockaddr* addr, socklen_t addrlen) {
        return Socket::bind(m_fd, addr, addrlen);
    }

    int connect(const SockAddr& sockaddr) {
        return connect(&static_cast<const struct sockaddr&>(sockaddr), SockAddr::Size);
    }

    int connect(const struct sockaddr* addr, socklen_t addrlen) {
        return Socket::connect(m_fd, addr, addrlen);
    }

    int accept(SockAddr& sockaddr) {
        return accept(&static_cast<struct sockaddr&>(sockaddr), SockAddr::Size);
    }

    int accept(struct sockaddr* addr, socklen_t addrlen) {
        return Socket::accept(addr, addrlen);
    }

    ssize_t send(const uint8_t* buf, size_t buflen, int flags) {
        return Socket::send(m_fd, buf, buflen, flags);
    }

private:
    static void init() {
        static std::once_flag once;
        std::call_once(once, []() {
#if defined(SOCKET_WINSOCK)
            WORD version = MAKEWORD(2, 2);
            WSADATA wsa_data;
            if (int err = WSAStartup(version, &wsa_data); err != 0) {
                LOG("FATAL: WSAStartup failed: " << err);
                throw std::runtime_error("WSAStartup failed");
            }
#endif
        });
    }

#if defined(SOCKET_WINSOCK)
    static int create_fd(SOCKET sock) { return _open_osfhandle(static_cast<intptr_t>(sock), 0); }
    static SOCKET get_socket(int fd) { return static_cast<SOCKET>(_get_osfhandle(fd)); }
    static int get_last_error() { return WSAGetLastError(); }
#else
    static int create_fd(int sock) { return sock; }
    static int get_socket(int fd) { return fd; }
    static int get_last_error() { return errno; }
#endif

    static int socket(int af, int type, int proto) {
        if (int fd = create_fd(::socket(af, type, proto)); fd == -1) {
            return -get_last_error();
        } else {
            return fd;
        }
    }

    static int bind(int fd, const struct sockaddr* addr, socklen_t addrlen) {
        return ::bind(get_socket(fd), addr, addrlen) != 0 ? -get_last_error() : 0;
    }

    static int accept(int fd, struct sockaddr* addr, socklen_t addrlen) {
        if (int fd = create_fd(::accept(get_socket(fd), addr, addrlen)); fd == -1) {
            return -get_last_error();
        } else {
            return fd;
        }
    }

    static int connect(int fd, const struct sockaddr* addr, socklen_t addrlen) {
        return ::connect(get_socket(fd), addr, addrlen) != 0 ? -get_last_error() : 0;
    }

    static int setsockopt(int fd, int level, int optname, void* optval, socklen_t optlen) {
#if defined(SOCKET_WINSOCK)
        auto optval_ = static_cast<const char*>(optval);
#else
        auto optval_ = optval;
#endif

        return ::setsockopt(get_socket(fd), level, optname, optval_, optlen) != 0 ? -get_last_error() : 0;
    }

    static ssize_t send(int fd, const uint8_t* buf, size_t buflen, int flags) {
#if defined(SOCKET_WINSOCK)
        auto buf_ = reinterpret_cast<const char*>(buf);
#else
        auto buf = static_cast<const void*>(buf);
#endif

        if (ssize_t res = ::send(get_socket(fd), buf_, buflen, flags); res == -1) {
            return -get_last_error();
        } else {
            return res;
        }
    }

    int m_fd{-1};
};

#endif // SOCKET_HPP
