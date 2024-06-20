#include <array>
#include <cassert>
#include <csignal>
#include <chrono>
#include <condition_variable>
#include <iostream>
#include <memory>
#include <mutex>
#include <thread>
#include <utility>

#include <omnicept/Glia.h>

#include "macros.hpp"
#include "omnicept.hpp"
#include "socket.hpp"

using namespace std::literals::chrono_literals;

using namespace HP::Omnicept;

static inline constexpr size_t k_mtu = 1400;
static inline constexpr uint32_t k_addr = INADDR_LOOPBACK;
static inline constexpr uint16_t k_port = 5000;
static inline constexpr float k_samplerate = 90000.0; // Hz

/// One-shot notification event.
class Event {
public:
    Event() {}
    ~Event() = default;

    Event(const Event&) = delete;
    Event& operator=(const Event&) = delete;

    operator bool() const { return m_set; }

    void set() {
        {
            std::lock_guard<std::mutex> lock(m_mutex);
            m_set = true;
        }
        m_condvar.notify_all();
    }

    void wait() {
        std::unique_lock<std::mutex> lock(m_mutex);
        m_condvar.wait(lock, [&]() { return m_set; });
    }

private:
    std::mutex m_mutex;
    std::condition_variable m_condvar;
    bool m_set{false};
};

static Event g_shutdown;

int main(int argc, char* argv[]) {
    // Create network socket
    auto sock = Socket<SockAddr<AF_INET>>::udp();
    SockAddr<AF_INET> sockaddr{k_addr, k_port};
    if (int res = sock.connect(sockaddr); res != 0) {
        LOG_ERROR("Failed to connect socket: " << res);
        return 1;
    }

    // Create Omnicept client
    auto license = std::make_unique<Abi::SessionLicense>("", "", Abi::LicensingModel::CORE, false);
    auto client = Glia::BuildClient_Sync("omnicept-facecam", std::move(license), [](const auto state) {
        LOG("Client state is " << state);
    });

    if (auto res = client->registerCallback<Abi::SubscriptionResultList>([&](auto result) {
        for (const auto& res : result->getSubscriptionResults()) {
            LOG("Subcription Result: " << res.result);
        }
    }); !success(res)) {
        LOG_ERROR("Failed to register SubscriptionResultList callback");
        return 1;
    }

    // Subscribe to Camera Image stream
    if (auto res = client->registerCallback<Abi::CameraImage>([&](auto image) {
        LOG("Got " << image->toString());

        if (image->imageData.size() != (image->width * image->height)) {
            LOG("skipping invalid frame");
            return;
        }

        const auto* data = reinterpret_cast<const uint8_t*>(image->imageData.data());

        static uint32_t seq = 0;
        uint32_t stride = image->width;
        size_t line_len = 2 * stride; // 16-bits per pixel
        auto ts = static_cast<uint32_t>((k_samplerate / image->framesPerSecond) * image->frameNumber);

        std::array<uint8_t, k_mtu> buf{0};

        for (uint16_t line = 0; line < image->height; line++) {
            bool last_line = (line == image->height - 1);
            auto it = std::begin(buf);

            // RTP header
            *it++ = 2 << 6; // version = 2
            *it++ = (last_line ? 0x80 : 0x00) | 0x7F; // End-of-Frame marker (1 bit) + Packet type (7 bit)
            *it++ = uint8_t(seq >> 8); // Sequence Number (2 bytes)
            *it++ = uint8_t(seq);
            *it++ = uint8_t(ts >> 24); // Timestamp (4 bytes)
            *it++ = uint8_t(ts >> 16);
            *it++ = uint8_t(ts >> 8);
            *it++ = uint8_t(ts);
            *it++ = 0x00; // SSRC (4 bytes)
            *it++ = 0x00;
            *it++ = 0x00;
            *it++ = 0x00;

            // Payload Header
            *it++ = uint8_t(seq >> 24); // Extended Sequence Number (2 bytes)
            *it++ = uint8_t(seq >> 16);

            // Payload Line Header
            *it++ = uint8_t(line_len >> 8); // Length (2 bytes)
            *it++ = uint8_t(line_len);
            *it++ = uint8_t(line >> 8) & 0x7F; // Field (1 bit) + Line Number (15 bits)
            *it++ = uint8_t(line);
            *it++ = 0x00; // Continuation (1 bit) + Offset (15 bits)
            *it++ = 0x00;

            size_t header_size = it - std::cbegin(buf);

            // UYVY
            for (size_t i = 0; i < stride; i++) {
                *it++ = 0x80; // alternating U/V
                *it++ = data[line * stride + i]; // Y
            }

            size_t size = it - std::cbegin(buf);
            assert(size == header_size + line_len);

            // Increment the packet sequence number
            seq++;

            if (ssize_t res = sock.send(buf.data(), size, 0); res <= 0) {
                LOG_ERROR("Unable to send packet: " << res);
                return;
            }
        }
    }); !success(res)) {
        LOG("Failed to register CameraImage callback");
        return 1;
    }

    auto subscriptions = Abi::SubscriptionList::GetSubscriptionListToNone();
    subscriptions->getSubscriptions().emplace_back(Abi::Subscription::generateSubscriptionForDomainType<Abi::CameraImage>());

    LOG("Setting subscriptions");
    if (auto res = client->setSubscriptions(*subscriptions); !success(res)) {
        LOG_ERROR("Set Subcriptions failed: " << res);
        return 1;
    }

    LOG("Starting Omnicept client");
    if (auto res = client->startClient(); !success(res)) {
        LOG_ERROR("Failed to start Omnicept client: " << res);
        return 1;
    }

    signal(SIGINT, [](int /* signum */) {
        LOG("Shutting down...");
        g_shutdown.set();
    });

    g_shutdown.wait();

    LOG("Disconnecting Omnicept client");
    if (auto res = client->disconnectClient(); !success(res)) {
        LOG("Error during disconnect: " << res);
        return 1;
    }

    return 0;
}
