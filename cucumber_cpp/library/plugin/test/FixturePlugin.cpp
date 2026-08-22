#include "cucumber_cpp/library/plugin/PluginExport.hpp"

// Minimal standalone fixture: exports one resolvable symbol and deliberately no
// ccr_register, so the loader's error paths can be exercised without a host that
// exports the runner's symbols.
extern "C" CCR_EXPORT int CcrTestFixtureAnswer()
{
    return 42;
}
