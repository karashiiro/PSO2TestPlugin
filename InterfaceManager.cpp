#include "InterfaceManager.h"

using namespace PSO2TestPlugin::Interface;

void InterfaceManager::Execute() {
    for (const auto del : delegates) {
        (*del)();
    }
}

void InterfaceManager::AddHandler(DrawFunc *delegate) {
    delegates.push_back(delegate);
}

[[maybe_unused]]
void InterfaceManager::RemoveHandler(DrawFunc *delegate) {
    for (auto it = delegates.begin(); it != delegates.end(); it++) {
        if (delegate == *it) {
            delegates.erase(it);
        }
    }
}
