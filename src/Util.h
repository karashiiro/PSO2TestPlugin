#pragma once

#define WIN32_LEAN_AND_MEAN
#include "Windows.h"

#include <sstream>
#include <string>

namespace PSO2TestPlugin::Util {
    /// \return A handle to the PSO2 game window.
    HWND GetGameWindowHandle();

    /// \brief User-defined to_string for the string type, used for variadic expansion.
    inline std::string const& to_string(std::string const& s) { return s; }

    /// \brief Concatenates strings.
    /// \return A fully-concatenated string.
    template<typename... Args>
    std::string JoinStrings(Args const& ... fragment) {
        using Util::to_string;
        using std::to_string;
        std::ostringstream agg;
        std::initializer_list<int> joiner{(agg << to_string(fragment), 0)...};
        static_cast<void>(joiner);
        return agg.str();
    }
}
