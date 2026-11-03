// SPDX-License-Identifier: GPL-3.0-or-later
// Links the complete lifted DKB modules to nxrt and checks their registry ABI without executing game code.
#include <cstdio>
#include "nxrt/game.h"
#include "nxrt/kernel.h"
#include "nxrt/nxmodule.h"
#include "nxrt/runtime.h"

extern "C" {
extern const NxModule nxmod_main, nxmod_sdk, nxmod_rtld;
extern const uint32_t nxmod_main_abi, nxmod_sdk_abi, nxmod_rtld_abi;
}

int main() {
    nxrt::game::Info identity;
    identity.id = "dkb";
    identity.title = "Donkey Kong Bananza";
    identity.product = "DKB module check";
    identity.program_id = 0x040074A01BF12000ull;
    identity.platform = nxrt::game::Platform::Switch2;
    nxrt::game::set_current(&identity);
    nxrt::InitOptions options;
    options.log_path = "dkb_module_check.log";
    options.watchdog = false;
    options.memguard = false;
    if (!nxrt::init(options)) return 1;
    struct Entry { const NxModule* module; uint32_t abi; };
    const Entry entries[] = {{&nxmod_main, nxmod_main_abi}, {&nxmod_sdk, nxmod_sdk_abi}, {&nxmod_rtld, nxmod_rtld_abi}};
    int result = nxrt::kernel::tick_hz() == 31250000ull ? 0 : 1;
    for (const auto& entry : entries) {
        const auto registration = nxrt::nxmodule_register(nullptr, entry.module, entry.abi);
        std::printf("%s: ABI %u, %zu functions, %s\n", entry.module->name, entry.abi, registration.functions,
                    registration.ok() ? "registered" : registration.message.c_str());
        if (!registration.ok() || registration.functions == 0) result = 1;
    }
    nxrt::shutdown();
    std::puts(result ? "DKB module check FAILED" : "DKB module check passed (game code was not executed)");
    return result;
}
