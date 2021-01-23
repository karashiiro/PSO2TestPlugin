#include "InterfaceManager.h"

using namespace PSO2TestPlugin::Interface;

void InterfaceManager::Execute() const {
    for (const auto &del : delegates) {
        (*del)();
    }
}

void InterfaceManager::AddHandler(DrawFunc *delegate) noexcept {
    delegates.push_back(delegate);
}

[[maybe_unused]]
bool InterfaceManager::RemoveHandler(DrawFunc *delegate) noexcept {
    for (auto it = delegates.begin(); it != delegates.end(); it++) {
        if (delegate == *it) {
            delegates.erase(it);
            return true;
        }
    }

    return false;
}
