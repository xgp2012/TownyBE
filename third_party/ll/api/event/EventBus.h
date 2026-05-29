#pragma once
#include <functional>
#include <type_traits>

namespace ll {
namespace event {

template<typename T>
class Event {
public:
    void registerHandler(std::function<void(T&)> handler) {}
};

class EventBus {
public:
    static EventBus& getInstance();
    template<typename T, typename Handler>
    void emplaceListener(Handler handler) {}
};

}
}
