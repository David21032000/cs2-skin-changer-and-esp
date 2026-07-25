#pragma once
#include <cstdint>

extern char g_dllDir[];

namespace Hooks {
    bool initialize();
    void shutdown();
}
