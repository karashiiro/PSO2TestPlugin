#pragma once

#include <vector>

namespace PSO2TestPlugin::Interface {
    typedef void(DrawFunc)();

    class InterfaceManager {
    public:
        /// \brief Runs all of the draw delegates the manager has.
        void Execute() const;

        /// \brief Adds a delegate to the manager.
        /// \param delegate The function to add.
        void AddHandler(DrawFunc *delegate) noexcept;

        /// \brief Removes a delegate from the manager.
        /// \param delegate The function to remove.
        [[maybe_unused]]
        bool RemoveHandler(DrawFunc *delegate) noexcept;
    private:
        std::vector<DrawFunc*> delegates;
    };
}
