#pragma once

#include <vector>

namespace PSO2TestPlugin::Interface {
    typedef void(DrawFunc)();

    class InterfaceManager {
    public:
        // Runs all of the draw delegates the manager has.
        void Execute();

        // Adds a delegate to the manager.
        void AddHandler(DrawFunc *delegate);

        // Removes a delegate from the manager.
        [[maybe_unused]]
        void RemoveHandler(DrawFunc *delegate);
    private:
        std::vector<DrawFunc*> delegates;
    };
}
