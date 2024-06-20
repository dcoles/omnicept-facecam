//! Helpers for Omnicept API.

#ifndef OMNICEPT_HPP
#define OMNICEPT_HPP

#include <omnicept/Glia.h>

namespace HP::Omnicept {

inline std::ostream& operator<<(std::ostream& os, const Client::State& state) {
    using State = Client::State;

    switch (state) {
        case State::PAUSED:
            os << "PAUSED";
            break;
        case State::RUNNING:
            os << "RUNNING";
            break;
        case State::DISCONNECTED:
            os << "DISCONNECTED";
            break;
        default:
            os << "Client::State(" << static_cast<int>(state) << ")";
    }

    return os;
}

inline std::ostream& operator<<(std::ostream& os, const Client::Result& result) {
    using Result = Client::Result;

    switch (result) {
        case Result::SUCCESS:
            os << "Success";
            break;
        case Result::ERROR_CLIENT_DISCONNECTED:
            os << "Disconnected";
            break;
        case Result::NO_OP_CLIENT_RUNNING:
            os << "Client Running";
            break;
        case Result::NO_OP_CLIENT_PAUSED:
            os << "Client Paused";
            break;
        case Result::UNSPECIFIED_ERROR:
            os << "Unspecified";
            break;
        default:
            os << "Client::Result(" << static_cast<int>(result) << ")";
    }

    return os;
}

inline std::ostream& operator<<(std::ostream& os, const Abi::SubscriptionResultType& result) {
    using Result = Abi::SubscriptionResultType;

    switch (result) {
        case Result::UNKNOWN:
            os << "Unknown";
            break;
        case Result::REJECTED:
            os << "Rejected";
            break;
        case Result::PENDING:
            os << "Pending";
            break;
        case Result::APPROVED:
            os << "Approved";
            break;
        default:
            os << "Abi::SubscriptionResultType(" << static_cast<int>(result) << ")";
    }

    return os;
}

inline std::ostream& operator<<(std::ostream& os, const Abi::SubscriptionResultErrorType& error) {
    using Error = Abi::SubscriptionResultErrorType;

    switch (error) {
        case Error::UNKNOWN_ERROR:
            os << "Unknown";
            break;
        case Error::SUCCESS_NO_ERROR:
            os << "Success (No Error)";
            break;
        case Error::NOT_LICENSED_ERROR:
            os << "Unlicenced";
            break;
        case Error::REJECTED_BY_USER_ERROR:
            os << "User Rejection";
            break;
        case Error::SPECIFIED_VERSION_NOT_AVAILABLE_ERROR:
            os << "Version Unavilable";
            break;
        case Error::MESSAGE_NOT_SUPPORTED:
            os << "Message Not Supported";
            break;
        case Error::LEGACY_SUBSCRIPTION_NOT_CHECKED:
            os << "Legacy Subscription";
            break;
        default:
            os << "Abi::SubscriptionResultErrorType(" << static_cast<int>(error) << ")";
    }

    return os;
}

inline std::ostream& operator<<(std::ostream& os, const Abi::CameraImage::ImageFormat& format) {
    using Format = Abi::CameraImage::ImageFormat;

    switch (format) {
        case Format::UNKNOWN:
            os << "UNKNOWN";
            break;
        case Format::RGB888:
            os << "RGB888";
            break;
        case Format::YUY2:
            os << "YUY2";
            break;
        case Format::UYVY:
            os << "UYVY";
            break;
        case Format::YVYU:
            os << "YVYU";
            break;
        case Format::YUYV:
            os << "YUYV";
            break;
        case Format::AYUV:
            os << "AYUV";
            break;
        case Format::YV12:
            os << "YV12";
            break;
        case Format::NV12:
            os << "NV12";
            break;
        case Format::L8:
            os << "L8";
            break;
        case Format::CUSTOM:
            os << "CUSTOM";
            break;
        default:
            os << "Abi::CameraImage::ImageFormat(" << static_cast<int>(format) << ")";
    }
    return os;
}

inline bool success(const Client::Result result) { return result == Client::Result::SUCCESS; }

}

#endif // OMNICEPT_HPP
