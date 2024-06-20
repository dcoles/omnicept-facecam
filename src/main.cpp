#include <chrono>
#include <condition_variable>
#include <iostream>
#include <memory>
#include <mutex>
#include <thread>
#include <utility>

#include <omnicept/Glia.h>

#include "omnicept.hpp"
#include "macros.hpp"

using namespace std::literals::chrono_literals;

using namespace HP::Omnicept;

int main(int argc, char* argv[]) {
    auto license = std::make_unique<Abi::SessionLicense>("", "", Abi::LicensingModel::CORE, false);
    auto client = Glia::BuildClient_Sync("omnicept-facecam", std::move(license), [](const auto state) {
        LOG("Client state is " << state);
    });

    std::mutex result_mutex;
    std::condition_variable result_cv;
    bool has_result = false;

    if (auto res = client->registerCallback<Abi::CameraImage>([&](auto result) {
        LOG("Got CameraImage");
    }); !success(res)) {
        LOG("Failed to register CameraImage callback");
        return 1;
    }

    if (auto res = client->registerCallback<Abi::SubscriptionResultList>([&](auto result) {
        for (const auto& res : result->getSubscriptionResults()) {
            LOG("Subcription Result: " << res.result);
        }

        {
            std::lock_guard<std::mutex> lock(result_mutex);
            has_result = true;
        }
        result_cv.notify_all();
    }); !success(res)) {
        LOG("Failed to register SubscriptionResultList callback");
        return 1;
    }

    auto subscriptions = Abi::SubscriptionList::GetSubscriptionListToNone();
    subscriptions->getSubscriptions().emplace_back(Abi::Subscription::generateSubscriptionForDomainType<Abi::CameraImage>());

    LOG("Setting subscriptions");
    if (auto res = client->setSubscriptions(*subscriptions); !success(res)) {
        LOG("Set Subcriptions failed: " << res);
        return 1;
    }

    LOG("Starting Omnicept client");
    if (auto res = client->startClient(); !success(res)) {
        LOG("Failed to start client: " << res);
        return 1;
    }

    {
        std::unique_lock<std::mutex> lock(result_mutex);
        result_cv.wait(lock, [&]() { return has_result; });
    }

    bool running = true;
    do {
        std::this_thread::sleep_for(30s);
    } while (running);

    LOG("Disconnecting Omnicept client");
    if (auto res = client->disconnectClient(); !success(res)) {
        LOG("Error during disconnect: " << res);
        return 1;
    }

    return 0;
}
