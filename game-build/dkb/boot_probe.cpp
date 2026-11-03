// SPDX-License-Identifier: GPL-3.0-or-later
// DKB SDK boot probe (headless): maps the three NSOs, registers the lifted modules, lets the host link them and
// enters rtld at offset 0 as the kernel would. No graphics. Structure follows game-build/src/pkp_main.cpp.
// Usage: dkb_boot_probe [--exefs DIR] [--max-seconds N]   (graphics build also: --window [--screenshot-at A,B,...])
// --window opens the game window (play mode: keyboard/pad input into the hid service, F1 overlay); the default stays headless.
#include <windows.h>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <string>
#include "nxrt/config.h"
#include "nxrt/dispatch.h"
#include "nxrt/game.h"
#include "nxrt/kernel.h"
#include "nxrt/log.h"
#include "nxrt/memory.h"
#include "nxrt/module.h"
#include "nxrt/nxmodule.h"
#include "nxrt/runtime.h"
#ifdef DKB_PROBE_GFX
#include <memory>
#include "nxrt/gfx.h"
#include "nxrt/nvn.h"
#include "nxrt/nvn_backend.h"
#include "nxrt/platform.h"
#include "nxrt/play.h"
const nxrt::play::GameDefaults* dkb_play_defaults();   // dkb_defaults.cpp
#endif

extern "C" {
extern const NxModule nxmod_main, nxmod_sdk, nxmod_rtld;
extern const uint32_t nxmod_main_abi, nxmod_sdk_abi, nxmod_rtld_abi;
}

namespace {
struct GameModule { const char* file; const NxModule* nx; const uint32_t* abi; uint64_t base; };
const GameModule kModules[] = {
    {"rtld", &nxmod_rtld, &nxmod_rtld_abi, 0x7700000000ull},
    {"main", &nxmod_main, &nxmod_main_abi, 0x7100000000ull},
    {"sdk", &nxmod_sdk, &nxmod_sdk_abi, 0x7500000000ull},
};
[[noreturn]] void die(const std::string& m) {
    NXLOG_E("boot", "%s", m.c_str());
    nxrt::log_flush();
    std::fprintf(stderr, "dkb_boot_probe: %s\n", m.c_str());
    TerminateProcess(GetCurrentProcess(), 1);
    for (;;) {}
}
#ifdef DKB_PROBE_GFX
std::unique_ptr<nxrt::gfx::Renderer>* g_renderer_slot = nullptr;
// --screenshot-at "A,B,..." : seconds after start at which the renderer writes dkb_shot_<n>.png of the presented frame.
std::string g_shot_times;
DWORD WINAPI shot_thread(LPVOID) {
    const ULONGLONG t0 = GetTickCount64();
    int n = 0;
    size_t i = 0;
    while (i < g_shot_times.size()) {
        size_t j = g_shot_times.find(',', i);
        if (j == std::string::npos) j = g_shot_times.size();
        const double at = std::atof(g_shot_times.substr(i, j - i).c_str());
        i = j + 1;
        const ULONGLONG due = t0 + static_cast<ULONGLONG>(at * 1000);
        while (GetTickCount64() < due) Sleep(50);
        if (g_renderer_slot && *g_renderer_slot) {
            char path[64];
            std::snprintf(path, sizeof path, "dkb_shot_%d.png", n++);
            (*g_renderer_slot)->request_screenshot(path);
            NXLOG_I("boot", "screenshot requested: %s", path);
        }
    }
    return 0;
}
#endif
DWORD WINAPI deadline_thread(LPVOID p) {
    Sleep(static_cast<DWORD>(reinterpret_cast<uintptr_t>(p)));
    NXLOG_W("boot", "max_seconds reached: guest still running\n%s", nxrt::summary().c_str());
    nxrt::log_flush();
    TerminateProcess(GetCurrentProcess(), 124);
    return 0;
}
}  // namespace

int main(int argc, char** argv) {
    nxrt::game::Info identity;
    identity.id = "dkb";
    identity.title = "Donkey Kong Bananza";
    identity.product = "DKB boot probe";
    identity.program_id = 0x040074A01BF12000ull;
    identity.platform = nxrt::game::Platform::Switch2;
#ifdef DKB_PROBE_GFX
    identity.play_defaults = dkb_play_defaults();
#endif
    nxrt::game::set_current(&identity);

    std::string exefs = std::getenv("DKB_EXEFS") ? std::getenv("DKB_EXEFS") : "";
    int max_seconds = -1;
    bool window = false, no_pads = false;
    for (int i = 1; i < argc; i++) {
        if (!std::strcmp(argv[i], "--exefs") && i + 1 < argc) exefs = argv[++i];
        else if (!std::strcmp(argv[i], "--max-seconds") && i + 1 < argc) max_seconds = std::atoi(argv[++i]);
#ifdef DKB_PROBE_GFX
        else if (!std::strcmp(argv[i], "--screenshot-at") && i + 1 < argc) g_shot_times = argv[++i];
#endif
#ifdef DKB_PROBE_GFX
        else if (!std::strcmp(argv[i], "--window")) window = true;
        else if (!std::strcmp(argv[i], "--no-pads")) no_pads = true;     // skip the gamepad subsystems (their start-up can take ~30 s with some HID devices)
#endif
        else { std::fprintf(stderr, "unknown argument %s\n", argv[i]); return 2; }
    }
    if (exefs.empty()) die("give --exefs DIR or set DKB_EXEFS");
    if (max_seconds < 0) max_seconds = window ? 0 : 30;

    nxrt::InitOptions opt;
    opt.log_path = "dkb_boot_probe.log";
    opt.watchdog = false;
    opt.memguard = false;
    if (!nxrt::init(opt)) die("runtime start-up failed");
    nxrt::config_set("process", "program_id", "0x040074A01BF12000");
#ifdef DKB_PROBE_GFX
    // The NVIDIA driver (nvgpucomp64.dll) crashes compiling the pre-raster pipeline-library part of large DKB vertex shaders
    // (e.g. 4a39a5198ae50adf, bdf65347359ab93b); whole pipelines compile fine. An explicit setting still wins.
    if (!nxrt::config_has("nvn", "gpl")) nxrt::config_set("nvn", "gpl", "false");
#endif

#ifdef DKB_PROBE_GFX
    // Headless NVN front end and Vulkan back end, as pkp.exe does for --headless (game-build/src/pkp_main.cpp).
    std::unique_ptr<nxrt::gfx::Device> device;
    std::unique_ptr<nxrt::gfx::Renderer> renderer;
    nxrt::nvn::install();
    {
        std::string err;
        nxrt::play::DisplayConfig disp;
        if (window) {
            disp = nxrt::play::display();      // controls.ini: window size, present mode, frame cap
            nxrt::platform::InitOptions po;
            po.gamepads = !no_pads;
            nxrt::platform::init(po);
            nxrt::platform::WindowDesc wd;
            wd.title = identity.title;
            wd.width = disp.width;
            wd.height = disp.height;
            if (!nxrt::platform::create_window(wd)) die("cannot create the game window (needs a Vulkan 1.3 driver)");
        }
        nxrt::gfx::DeviceDesc dd;
        dd.headless = window ? 0 : 1;
        device = nxrt::gfx::Device::create(dd, &err);
        if (!device) die("graphics device: " + err);
        nxrt::gfx::RendererDesc rd;
        rd.headless_width = 1280;
        rd.headless_height = 720;
        if (window) rd.present_mode = disp.present_mode;
        renderer = nxrt::gfx::Renderer::create(*device, rd, &err);
        if (!renderer) die("renderer: " + err);
        nxrt::nvn::BackendDesc bd;
        bd.device = device.get();
        bd.renderer = renderer.get();
        if (window) {
            bd.limiter = nxrt::play::limiter();
            bd.keep_aspect = disp.keep_aspect;
        }
        if (!nxrt::nvn::install_backend(bd)) die("NVN back end failed to start");
        g_renderer_slot = &renderer;
        if (!g_shot_times.empty()) CreateThread(nullptr, 0, shot_thread, nullptr, 0, nullptr);
        nxrt::play::StartOptions po;
        po.device = device.get();
        po.renderer = renderer.get();
        po.window = window;
        po.overlay = window;
        nxrt::play::start(po);
    }
#endif
    for (const GameModule& g : kModules) {
        nxrt::LoadOptions lo;
        lo.name = g.file;
        lo.base = g.base;
        std::string err;
        const std::string path = exefs + "\\" + g.file;
        nxrt::Module* m = nxrt::module_load_nso_file(path.c_str(), lo, &err);
        if (!m) die("cannot load " + path + ": " + err);
        if (m->base() != g.nx->base) die(std::string("base mismatch for ") + g.file);
        const nxrt::RegResult r = nxrt::nxmodule_register(m, g.nx, *g.abi);
        if (!r.ok()) die(std::string("lifted module ") + g.file + " refused: " + r.message);
        NXLOG_I("boot", "%s mapped at 0x%llx, %zu functions", g.file, (unsigned long long)g.base, r.functions);
    }
#ifdef DKB_PROBE_GFX
    // The game reaches NVN only through nvnBootstrapLoader, defined by the sdk (Nintendo's driver); use the runtime's front end.
    nxrt::module_override_symbol("nvnBootstrapLoader");
    if (const uint64_t def = nxrt::nxmodule_find_export("nvnBootstrapLoader"))
        if (nxrt::HleFn fn = nxrt::hle_lookup("nvnBootstrapLoader")) nxrt::fn_register(def, fn);
#endif
    nxrt::module_protect_all();   // the game's own rtld relocates, as on the console
    if (max_seconds > 0) CreateThread(nullptr, 0, deadline_thread, reinterpret_cast<void*>(static_cast<uintptr_t>(max_seconds) * 1000), 0, nullptr);

    const uint32_t h = nxrt::kernel::main_thread_adopt(1u << 20, "main");
    if (!h) die("cannot create the main thread");
    ::Ctx* ctx = nxrt::kernel::current_ctx();
    if (!nxrt::fn_lookup(kModules[0].base)) die("the lifted rtld has no function at its entry");
    ctx->x[0] = 0;
    ctx->x[1] = h;
    ctx->x[30] = 0;
    NXLOG_I("boot", "entering rtld at 0x%llx", (unsigned long long)kModules[0].base);
    nxrt::dispatch_call(ctx, kModules[0].base);
    NXLOG_I("boot", "entry returned x0=0x%llx", (unsigned long long)ctx->x[0]);
    nxrt::kernel::main_thread_release();
    nxrt::shutdown();
    return static_cast<int>(ctx->x[0] & 0xff);
}
