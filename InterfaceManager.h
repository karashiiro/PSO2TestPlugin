#pragma once

#include <functional>
#include <vector>

class InterfaceManager {
public:
    explicit InterfaceManager();

    // Runs all of the draw delegates the manager has.
    void Execute();

    // Adds a delegate to the manager.
    void AddHandler(const std::function<void()> &delegate);

    // Removes a delegate from the manager.
    [[maybe_unused]]
    void RemoveHandler(const std::function<void()> &delegate);
private:
    std::vector<std::reference_wrapper<const std::function<void()>>> delegates;
};
