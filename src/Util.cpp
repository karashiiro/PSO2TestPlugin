#include "Util.h"

using namespace PSO2TestPlugin;

HWND Util::GetGameWindowHandle() {
    return FindWindow("PHANTASY STAR ONLINE 2 NEW GENESIS closedβtest", nullptr);
}
