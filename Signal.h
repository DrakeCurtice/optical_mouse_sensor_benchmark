#include <iostream>
#include <algorithm>
#include <string>



#pragma once

#include <vector>
#include <functional>
#include <type_traits>
#include <utility>

#include "Slot.h"

template <typename Event>
class Signal {
public:
    void connect(Slot<Event>& slot) {
        // TODO: Connect a slot
    }

    void emit(const Event& event) {
        // TODO: Notify connected slots
    }

private:
    std::vector<Slot<Event>*> slots_;
};