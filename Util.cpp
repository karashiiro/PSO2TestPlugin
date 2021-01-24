#include "Util.h"

using namespace PSO2TestPlugin;

HWND Util::GetGameWindowHandle() {
    return FindWindow("Phantasy Star Online 2", nullptr);
}
