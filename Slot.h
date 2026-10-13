#pragma once

#include <functional>
#include <utility>

template <typename Event>
class Slot {
public:
    using Callback = std::function<void(const Event&)>;

    explicit Slot(Callback callback)
        : callback_(std::move(callback)) {}

    void run(const Event& event) {
        // TODO: Execute callback
    }

private:
    Callback callback_;
};