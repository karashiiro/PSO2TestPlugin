#include "InterfaceManager.h"

InterfaceManager::InterfaceManager() = default;

void InterfaceManager::Execute() {
    for (const auto& del : delegates) {
        del();
    }
}

void InterfaceManager::AddHandler(const std::function<void()> &delegate) {
    delegates.push_back(std::ref(delegate));
}

[[maybe_unused]]
void InterfaceManager::RemoveHandler(const std::function<void()> &delegate) {
    for (auto it = delegates.begin(); it != delegates.end(); it++) {
        if (&delegate == &(*it).get()) {
            delegates.erase(it);
        }
    }
}
