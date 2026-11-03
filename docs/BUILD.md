# Build results and handoff — 2026-11-03

## Resume here

Workspace: `C:/Users/Marek/Desktop/DKBRecomp`, branch `codex/dkb-prototype`.
Upstream Pokopia base: `2c29515b23a6d78d967775fd7e9f24f50402cca7`.
The user stopped further implementation after asking to save documentation.
No commits or pushes were created. Source changes are still uncommitted; inspect
`git status` before continuing. Keep EGLANTINE and the old backup unchanged.

**The build milestone is complete. DKB SDK startup and gameplay are not implemented.**
The next useful step is a DKB runtime profile and an explicit SDK boot probe, not
renaming the Pokopia launcher or treating the module checker as a playable game.

## Built and checked

| Output | Result |
|---|---|
| `build/runtime-core/nxrt.lib`, `nxrt_tests.exe` | Headless runtime and tests compile/link |
| `tools/textures/build/release/astc2bc7.exe` and libraries | Texture tools compile/link |
| `build/runtime-gfx/nxrt_gfx_demo.exe` | SDL/Vulkan runtime compiles/links |
| `work/build/dkb-llvm23/dkb_main.lib` | DKB main, 143,086 generated functions |
| `work/build/dkb-llvm23/dkb_sdk.lib` | DKB sdk, 47,874 generated functions |
| `work/build/dkb-llvm23/dkb_rtld.lib` | DKB rtld, 37 generated functions |
| `build/dkb-probe/dkb_module_check.exe` | Links all three complete DKB libraries to the real headless runtime |

The generated C++ reports zero untranslated instructions for all three modules.
The native checker ran successfully on Windows, checked the Switch 2 tick frequency,
and registered **190,997 functions with ABI 3**. It did not map the guest images,
run constructors, execute guest functions or start the SDK. Its runtime log reports
zero SVC/IPC calls and no executed game frames. The executable contains game-derived
code and remains an ignored local build artifact.

The graphics demo ran headlessly on the NVIDIA GeForce RTX 5070 Ti for three frames
at 640x360, with **zero Vulkan validation errors and zero warnings**. The screenshot
`build/runtime-gfx/smoke.png` was viewed and showed the expected coloured test pattern.
This is the framework's demo scene, not DKB graphics; its FPS is not a DKB measurement.

## Tests and remaining failures

The complete inherited core suite was run with the registered **Pokopia** test profile:
`ctest --test-dir build/runtime-core --output-on-failure --parallel 1`.
Result: **62/73 groups passed, 11 failed**, approximately 48 seconds.

Failed groups: `kernel_threads`, `media_movie`, `svc_acc`, `svc_am`, `svc_common`,
`svc_fs`, `svc_info`, `svc_memory`, `svc_misc`, `svc_settings`, `tt_core`.

Some failures demonstrably expect old Metroid Prime facts despite the Pokopia profile:
program ID `0x010012101468C000`, save sizes, languages, core masks and timing assumptions.
`media_movie` expects the old movie interception table under a Switch 2 profile.
Other failures, including test-tooling/probe behavior, remain unresolved. Do not
describe the suite as passing or change expectations blindly to obtain green output.

The corrected clock case was also run alone and passed:
`nxrt_tests.exe kernel_threads.system_tick_monotonic_and_rate` (1/1).
The full graphics test suite, lifted synthetic integration suite, shader translation
pipeline and DKB gameplay were not run. libvpx was not found: this runtime build
does not provide the host VP9/VP8 decoder, and the checker logs that movies are left out.

## Source changes made for the build

- `runtime/scripts/env.cmd`: discover VS C++ Build Tools with vswhere, use x64, prefer
  the project-local LLVM 23 toolchain, fall back to installed LLVM/VS LLVM.
- `recomp/scripts/nx_env.cmd`: reuse that environment; sccache is optional.
- `recomp/cmake/clang-cl.cmake`: discover LLVM tools rather than requiring one global path.
- `runtime/scripts/build.ps1`: optional sccache no longer forced; select existing `pkp`
  instead of absent `mpr`, add `-CoreOnly` / `-Jobs`, and bound the clean path.
  Its PowerShell syntax was checked; the validated builds used direct CMake commands.
- `runtime/CMakeLists.txt`, `src/core/switch2.cpp`, `src/testtools/tt_core.cpp`: guard
  renderer-only HDR/screenshot-worker calls when graphics is disabled. This fixes
  unresolved symbols in a headless link while retaining them in graphics builds.
- `runtime/tests/test_kernel_threads.cpp`: replace a removed `kTickHz` constant with
  an expected frequency selected by platform; scale the timing bounds accordingly.
- `recomp/games/dkb/`: recompiler profile, build-ID/NSO-segment verification and CMake
  targets for main/sdk/rtld. No Pokopia hooks or patch sites are applied to DKB.
- `game-build/dkb/`: diagnostic link/ABI/registry checker; this is not the game's entry point.

There is still no `runtime/games/dkb/` integration. The checker's local identity exists
only to select Switch 2 behavior for its limited test.

## Local toolchain and inputs

- Visual Studio 2022 Build Tools: `C:/Program Files (x86)/Microsoft Visual Studio/2022/BuildTools`.
- Initial runtime, graphics and texture builds used bundled Clang 19.1.5.
- LLVM 19 crashed in its greedy register allocator compiling generated main `c_072.cpp`,
  function `f_7100ca00e0`. Logs and crash context remain local. Do not edit generated C++
  to conceal this compiler crash.
- LLVM **23.1.4** was downloaded from the official LLVM release and extracted under
  `build/toolchain/clang+llvm-23.1.4-x86_64-pc-windows-msvc/`. Its SHA-256 was verified:
  `854bffcfc84528e0ee92490ff5c178920abd358a79c3e98f053e41e3e8e4c6d2`.
  It successfully compiled the same file, all DKB modules and the native checker.
  DKB CMake now requires LLVM 23 or newer. The failed LLVM 19 build remains in
  `work/build/dkb/`; the successful build is `work/build/dkb-llvm23/`.
- `tools/.venv/` is installed with Python 3.12 and `recomp/requirements.txt`:
  Capstone 5.0.9, Unicorn 2.1.4, LZ4 4.4.5, pyelftools 0.33. This environment built
  these modules successfully; upstream's general build documentation names Python 3.13.
- Texture dependencies were fetched by the pinned upstream script. SDL3, Vulkan
  headers, volk, VMA and ImGui were fetched and hash-checked by upstream CMake.
- No .NET SDK was installed; the Ampere shader decoder project targets .NET 10.

DKB input (read-only):
`C:/Users/Marek/Desktop/EGLANTINE/GAMES/DKB ROM/updates/458752/exefs`.
The recompiler profile checks these NSO build IDs (zero-padded field omitted):

| Module | Build ID |
|---|---|
| main | `e25c90c1c136638885659fd55bf8a03b3403489d` |
| sdk | `18265daea2105b31f25346adfbbd09b5edf1207f` |
| rtld | `cc38c02463e7dd1136ccf03f2a31c39dbe0758ed` |

All three modules' uncompressed segment hashes passed verification. Layout/index:
main `0x7100000000` / 0, sdk `0x7500000000` / 1, rtld `0x7700000000` / 2.
The recompiler adds rtld entry offset 0. The old TotK context/layout/objects must not
be linked into this implementation.

## Rebuild or rerun on this machine

From the repository root, in PowerShell; the CMake caches and dependencies already exist:

```powershell
cmd /d /c 'call runtime\scripts\env.cmd && cmake --build build/runtime-core --parallel 8'
cmd /d /c 'call runtime\scripts\env.cmd && cmake --build build/runtime-gfx --target nxrt_gfx_demo --parallel 6'
cmd /d /c 'call recomp\scripts\nx_env.cmd cmake --build work/build/dkb-llvm23 --target dkb_modules --parallel 8'
cmd /d /c 'call recomp\scripts\nx_env.cmd cmake --build build/dkb-probe --parallel 4'
```

To create the DKB module and checker build folders again:

```powershell
cmd /d /c 'call recomp\scripts\nx_env.cmd cmake -S recomp/games/dkb -B work/build/dkb-llvm23 -G Ninja -DCMAKE_TOOLCHAIN_FILE="C:/Users/Marek/Desktop/DKBRecomp/recomp/cmake/clang-cl.cmake" -DCMAKE_BUILD_TYPE=Release -DDKB_EXEFS="C:/Users/Marek/Desktop/EGLANTINE/GAMES/DKB ROM/updates/458752/exefs"'
cmd /d /c 'call recomp\scripts\nx_env.cmd cmake -S game-build/dkb -B build/dkb-probe -G Ninja -DCMAKE_TOOLCHAIN_FILE="C:/Users/Marek/Desktop/DKBRecomp/recomp/cmake/clang-cl.cmake" -DCMAKE_BUILD_TYPE=Release'
```

Build the module libraries before configuring the checker. Its `DKB_CORE_LIB` defaults
to `build/runtime-core/nxrt.lib` and `DKB_LIB_DIR` to `work/build/dkb-llvm23/`.

Run diagnostics with their own build folder as the working directory:

```powershell
Push-Location build/dkb-probe
./dkb_module_check.exe
Pop-Location
Push-Location build/runtime-gfx
./nxrt_gfx_demo.exe --headless --frames 3 --size 640x360 --shot-frame 1 --screenshot smoke.png --validation 1
Pop-Location
```

## Evidence and next work

Logs are in ignored `build/logs/`: `runtime-core-configure.log`, `runtime-core-build.log`,
`runtime-core-tests.log`, `clock-test.log`, `textures-build.log`,
`runtime-gfx-configure.log`, `runtime-gfx-build.log`, `gfx-smoke.log`,
`dkb-build.log` (LLVM 19 failure), `dkb-llvm23-configure.log`, `dkb-llvm23-build.log`,
`dkb-probe-configure.log`, `dkb-probe-build.log`, `dkb-module-check.log`.
Artifact sizes, SHA-256 values and executable AMD64 checks are recorded in
`build/build-result-20261103.json`. The checker also writes
`build/dkb-probe/dkb_module_check.log`.

Next: add the actual DKB runtime profile, map/load its images and validate SDK
initialization/constructors with build-identified interception sites. Then handle
DKB shader containers and the upscaler/terrain gaps described in
[dkb-bringup.md](dkb-bringup.md) and [pokopia-evaluation.md](pokopia-evaluation.md).
The old nxnative reference is in the separate backup's `refs/nxnative/`.

Keep game inputs, generated code, binaries, toolchains, environments and logs ignored.
No source changes to EGLANTINE or the backup were made during this build task.

## Boot probe result — 2026-11-03 (later)

`game-build/dkb/boot_probe.cpp` (`build/dkb-probe/dkb_boot_probe.exe`, headless, no NVN front end)
maps rtld/main/sdk, registers them and enters rtld at offset 0. Build with the two
`nx_env.cmd cmake ...` commands above from PowerShell (not from Git Bash). Run with
`NXRT_FS_ROMFS` set to the update's `romfs` (read in place) and `--exefs <update exefs>`.

- Without a RomFS folder the guest stops in `svcBreak` (fs result module 2, description 1002).
- With RomFS the guest completes rtld linking, SDK start-up and reaches game code: about 0.4 s,
  hundreds of SVCs, 0 unknown SVCs, 0 unhandled IPC commands, one RomFS file opened
  (`/System/HeapSize/Product.Nin_Ounce_NVN2.HeapDef.byml.zs`).
- It then faults with a null read in sdk `f_75005776a0` (x0 = 0). The registers hold the string
  `/dev/nvhost-tsg...`: this is the SDK's own NV driver being opened. Pokopia avoids it by forcing
  `nvnBootstrapLoader` to the runtime's NVN front end (`nxrt::nvn::install()`, needs `nxrt_gfx`).
  The probe links only the headless core, so it cannot do that yet.

Next: link the probe (or a new `dkb.exe`) against `build/runtime-gfx` (`nxrt_gfx`), call
`nxrt::nvn::install()` and `module_override_symbol("nvnBootstrapLoader")`, then rerun.
This is not evidence of gameplay or rendering.

## Boot probe with NVN (headless) — 2026-11-03 (later still)

`build/dkb-probe/dkb_boot_probe_gfx.exe` is the same probe linked with `build/runtime-gfx` (`nxrt_gfx`),
the runtime's NVN front end and a headless Vulkan back end on the RTX 5070 Ti; `nvnBootstrapLoader` is
forced to the runtime as in Pokopia. Build target `dkb_boot_probe_gfx` (PowerShell, nx_env.cmd).

- The guest now runs ~8.7 s, ~1,150+ SVCs, 0 unknown SVCs, reads RomFS (audio and system data), creates
  an NVN device, and issues texture/shader work. No frame is presented.
- `IAudioDevice` commands 19 and 20 (Switch 2 SDK, undocumented) were not implemented and aborted via
  `svcBreak`. `runtime/src/services/audio.cpp` now answers both with success and no output, logging each
  call as a warning. **These answers are unverified guesses**; replace them once the semantics are known.
- It then stops with `GUEST TRAP: indirect call to an address that is not a known function (0x0)`
  from main `f_7100203674` (lr 0x71002036f8). Not yet diagnosed. Candidates: one of the ~384 NVN procs the
  game requests by name that the table lacks (logged as `UNKNOWN NVN PROC`, e.g. nvnProgramBuilder*,
  mesh-task, ray-tracing, fast-clear), or a missing service.
- `nvnProgramSetShaders: no translated shader for block ...`: no DKB shader cache exists (BNSH/BAHS/NDSR
  extraction is still milestone 4). Texture transcoding failed because `astcdec-*-shared.dll` is not next
  to the exe (copy from `tools/textures/build/release`).
- `game_info` has no timestep/play tables, so play mode is not pumped.

## Null-call diagnosis and 60 s run — 2026-11-03 (end of day)

The `GUEST TRAP ... (0x0)` was nn::vi: the sdk's `nn::vi::SetLayerNextHdrContentGamutMetadata` (and then
`...HdrToSdrToneMappingMetadata`) dispatches through a backend object that the skipped GPU boot never
fills. `runtime/src/core/switch2.cpp` now answers these DKB-used vi imports on the host: gamut and
tone-mapping metadata, extended swap interval (set/clear) with nn::Result 0, and display queries reporting
no HDR, no VRR and 203 nits paper white. These host answers are choices, not measured console behaviour.

Result of `dkb_boot_probe_gfx` (headless, RTX 5070 Ti, `astcdec-*.dll` beside the exe, update RomFS):
the guest runs the full 60 s limit with no crash. 35.8 M SVCs, 0 unknown. Frames are presented headlessly
(~760 per second, i.e. almost nothing is drawn: 2,721 `no translated shader for block` warnings, since DKB
shader extraction does not exist). Remaining visible gaps: `IManagerForApplication` cmd 130 unimplemented,
`nvn.replay: CallCommands of an unknown handle skipped`, ~384 NVN procs requested by name that the table lacks,
the two guessed `IAudioDevice` answers above, no libvpx. Not a rendering or gameplay result.

PowerShell note: run the `nx_env.cmd` commands from PowerShell; Git Bash mangles `cmd /d /c` and builds nothing.

## DKB shader blocks dumped from the running game — 2026-11-03

Instead of parsing `Shader/*.bfsha.zs` first, the runtime's `[nvn] dump_missing` (`NXRT_NVN_DUMP_MISSING`)
was used: 45 s of `dkb_boot_probe_gfx` wrote **2,721 distinct shader blocks** (1,515 vertex, 1,167 fragment,
39 compute) with `manifest.tsv` in the harness layout to ignored `work/shaders/dkb_blocks/`. These are only
the shaders requested during boot, not the whole game (RomFS `Shader/` holds `Project.Nin_Ounce_NVN2.release.sarc.zs`
and two `.bfsha.zs` archives, 14 MB, still unparsed).

Blocker for translation: no .NET SDK is installed (only runtime 8.0.22; the Ampere decoder and harness need
SDK 10) and `refs/ryubing` (Ryubing, Codeberg archive `a82350bb77`) is absent. Vulkan SDK 1.4.357 with
`spirv-val` is present. Whether the existing Ampere frontend handles DKB's HMMA/LDSM/MOVM upscaler shaders
is unmeasured.

## Shader translation and first DKB frames — 2026-11-03 (night)

Tools set up locally (all ignored by Git): .NET SDK 10.0.403 via Microsoft's `dotnet-install.ps1` into
`build/toolchain/dotnet`; Ryujinx **1.3.2** (MIT) sources copied from the old backup's
`tools/vendor/ryujinx` into `tools/shaders/third_party/ryubing` (the README names a Ryubing archive,
`a82350bb77`, which was unavailable; the harness patches applied and built against 1.3.2, but results were
not compared with that revision). Build: `dotnet build tools/shaders/harness/SpirvHarness.csproj -c Release`
with `DOTNET_ROOT` and `PATH` pointing at the local SDK.

`spirv_harness --batch work/shaders/dkb_blocks --out work/shaders/dkb_spirv --jobs 8` (13 s, `spirv-val`
vulkan1.3): **2,699 of 2,721 blocks ok** (compute 18, fragment 1,166, vertex 1,515); 21 compute blocks fail
(`Invalid storage kind GlobalMemory`: global-memory Store/AtomicAdd) and 1 fragment block has an unhandled
opcode (`.UNKNOWN_526`). The 2,721 blocks cover only what boot requested. HMMA/LDSM/MOVM upscaler shaders
were not among them; that gap is still unmeasured.

Run the probe with `NXRT_NVN_SHADER_CACHE=work/shaders/dkb_spirv` and `NXRT_FS_ROMFS=<update romfs>`
(set both in the same PowerShell call as the exe) and `--screenshot-at 15,30,45`:
the headless renderer produced real DKB frames: a notice/prompt screen and a menu-style panel (yellow banner,
cyan/magenta outlined box). **Text glyphs are solid green blocks** (font/atlas texture or sampling problem,
undiagnosed). Everything is a headless 50 s boot probe: no input, no gameplay, no performance claim.

Stability: the run is not deterministic. Observed failures: with pipeline libraries on, an access violation
inside the NVIDIA driver's `nvgpucomp64.dll` on the `nvn-gpl` thread (one of the translated SPIR-V modules,
unidentified); with `NXRT_NVN_GPL=0 NXRT_NVN_PREWARM=0`, one run crashed in runtime code reading address 0x28
(guest `f_71001114a0`) and the next ran the full 50 s. Neither is diagnosed. `dkb_boot_probe_gfx` now links
with `/MAP` so host frames can be named. The probe takes `--screenshot-at`.

## Fix: text drawn as solid blocks — 2026-11-03 (Ampere TEX half results)

Cause (found with `NXRT_NVN_TRACE_SEC`/`TRACE_SNAP` and the SASS dump): the glyph atlas (R8, 1024x8192) and the text draw
were correct; the three alpha-tested text fragment shaders (`7f55d1b5389e14d6`, `ae4693c613fd2fdf`, `4fbad69e51fa58c0`)
use a `TEX.SCR ... ARRAY_2D` with **bit 79** of the instruction set and read the result as packed halves
(`TEX.SCR RZ, R4, ... 0x8` then `HADD2.F32 R8, -RZ, R4.H0_H0`). The decoder ignored the bit and the front end stored the
sample as 32-bit floats, so alpha was garbage. Only 3 of 10,531 sampling instructions in the 2,721 dumped blocks use
the bit, all this pattern.

Change: `tools/shaders/ampere/decoder/SassDecoder.Tex.cs` decodes bit 79 as modifier `F16` on TEX;
`tools/shaders/ampere/frontend/AmpereEmit.cs` (`EmitTexAmpere`) packs the sampled components into half2 pairs
(components 0/1 into the first destination, 2/3 into the second). **Inferred, not confirmed by nvdisasm**: the
spelling `F16` is mine, and the oracle (`sass_oracle.py --verify`) was not rerun, so its locked baseline is unchecked.
Translation totals are unchanged (2,699 ok / 21 compute exceptions / 1 unsupported).

Result: the headless probe now shows legible DKB text: the autosave notice ("When you see this banana icon…") and a menu
with "Assist Mode" and a "+ Options" prompt. The yellow banner above the menu still shows no text (unexplained). Still
headless, no input, no gameplay.

## Crash triage — 2026-11-03 (end)

1. **NVIDIA driver crash (deterministic with pipeline libraries on).** With `[nvn] gpl` at its default (true), 6 of 6
   runs died ~2 s in with an access violation at `nvgpucomp64.dll+0x3c1ca00` on the `nvn-gpl` thread while compiling the
   *pre-raster part* of a vertex shader. Logging the shader modules of each part (temporary, removed) named
   `4a39a5198ae50adf`; with it removed from the cache the crash moved to `bdf65347359ab93b`. Both are large DKB vertex
   shaders (56-76 KB SPIR-V, `cb8`/`cb9`/`cb10`, DrawParameters, `spirv-val` clean). The trigger inside the SPIR-V is not
   identified: this is a driver compiler fault on valid input, seen on the RTX 5070 Ti with the installed driver only.
   With `NXRT_NVN_GPL=0` (whole pipelines, prewarm still on) 4 of 4 runs of 45 s and 3 of 3 of 60 s were clean.
   `dkb_boot_probe_gfx` therefore sets `[nvn] gpl = false` unless it is set explicitly. Cost: slower first draws.
2. **Runtime null read (0x28)**, seen once in a GPL-off run before the TEX fix and the `/MAP` link: not reproduced in
   13 later GPL-off runs (6 x 25 s, 4 x 45 s, 3 x 60 s). Unexplained; may have been a consequence of the garbage text
   shader output. Do not treat as fixed.
3. Still open: `IManagerForApplication` cmd 130, the two guessed `IAudioDevice` answers, 21 compute shaders using
   global-memory atomics, the 384 NVN procs the table lacks.

Everything above is for the 60 s headless boot probe; no input, no gameplay.

## Compute shaders with global-memory atomics — 2026-11-03 (end)

The 21 failing compute blocks (`Failed to reserve storage buffer for global memory operation "AtomicAdd"/"Store"`) were
not an atomics or STG gap. Example `3d1ddc3fbb3d4afc` loads 17 distinct 64-bit pointers from `c[0x0][0x60..0x1e0]` (`ULDC.64`
then `LDG`/`STG`/`ATOMG [UR+R.U32]`); Ryujinx's `ResourceManager` has `_sbSlotToBindingMap = new SetBindingPair[16]`, so the 17th
pointer cannot get a storage buffer (loads from it were already lost silently in the shaders that "worked"... not
verified; only the Store/AtomicAdd failures are reported).

Change: `tools/shaders/harness/translator/spirv-storage-buffers-32.patch` (+ csproj entries) raises the slot count to 32
(the written-slots mask is already a `uint`). Result on the 2,721 dumped blocks: **2,720 ok** (compute 39/39, fragment 1,166,
vertex 1,515), 1 unsupported fragment (`.UNKNOWN_526`). Highest new storage binding: 16. The runtime's compute table
mapping (`apply_nvn2_compute_table`) is generic over the control section's counts; with the new cache a 45 s probe run
logged no pipeline/descriptor errors, one block still missing, identical frames. Not verified: that the 17th buffer is bound
with correct contents (no compute-output check was made), Vulkan validation layers were not run, and vertex shaders
with more than 16 storage buffers would now share a binding range with the fragment stage (none seen).

## Missing banner text and whole-game shader containers — 2026-11-03 (end, in progress)

**Banner text (reported by the user: white text should be on the yellow banner).** Findings so far, headless probe, traced frame:
- The one untranslated block (`809228fb1c2e9cc4`, `.UNKNOWN_526` = an IPA-like attribute fetch with a register operand, samples a cube-map array)
  is a 3D-scene shader, not the banner text. It is not the cause.
- Text draws in the traced frame: #61 (`vs c92c4a36057f7404`, `fs 7f55d1b5389e14d6`, 11 glyph instances, R8 atlas, swizzle 1112),
  #63 and #65 (`vs 82c00ee93c7a4d18`, same fs: "Assist Mode", "+ Options", both visible), #64 (`vs c92c...`). The draws that go through
  `vs c92c...` (storage-buffer read `sb[c0+0x44]`, query `unalignedStorage`) are the ones whose glyphs are missing; the `vs 82c0...` draws work.
  Before the TEX fix the banner showed dark boxes at the right positions, so the glyph quads exist and are placed.
  Hypotheses, not tested: alpha or colour read from the unaligned storage buffer is wrong for `vs c92c...`; or a blend/ordering issue. Next step:
  snapshot the target before and after draw #61 in one frame and dump the instance data.

**Whole-game shaders.** Containers decompressed with Python 3.14's `compression.zstd` (no download) into ignored `work/shaders/dkb_containers/`:
`SilverStarMaterial.Product.200...bfsha` (88.7 MB, `FSHA`), `PhotoFilter...bfsha` (70 KB), `Project.Nin_Ounce_NVN2.release.sarc` (10.8 MB, `SARC`).
The SARC holds 8 files: `ActionLibraryPass.sharcb` (5.6 MB, `BAHS`), `ActionLibraryProcedural.sharcb` (3.1 MB, `BAHS`),
`SilverStarPass.sharcb` (160 KB, `BAHS`), `SilverStarMaterial.Nin_Ounce_NVN2.bfsha` (1.9 MB, `FSHA`) and AAMP config files. The BFSHA/BAHS parsers
and the extractor to the harness layout are not written yet; nothing beyond the 2,721 boot-time blocks is translated.

## Fix: banner text (LDC.64 with an overlapping index register) — 2026-11-03 (end)

Cause, found by tracing the text draws: draws through `vs c92c4a36057f7404` (glyph instances read from a storage buffer) changed no pixel
(snapshots after draws #60 and #61 identical) although the atlas held glyph data under the instances' UVs (checked by dumping the sampled R8
texture) and the instance colour was 0xffffffff. In the vertex shader's SASS, `LDC.64 R12, c[0x1][R12]` uses the index register as the
destination pair. `EmitLdcAmpere` wrote word 0 into R12 before loading word 1, so the second load indexed with the first value and the
texture coordinate was garbage (the SPIR-V showed an index stored where a loaded value belonged).

Change: `tools/shaders/ampere/frontend/AmpereEmit.cs` (`EmitLdcAmpere`) now loads all words first, then writes the destinations.
139 of the 2,721 translated shaders change; totals are unchanged (2,720 ok, 1 unsupported) and all still pass `spirv-val`.
Result in the headless probe: the yellow banner now reads "Normal Mode" (dark text on yellow) and a line "The standard way to play." appears under the
menu. Text on the banner is dark, not white; that is what the game draws here (I did not verify against a console).

Also added: the draw trace (`NXRT_NVN_TRACE_SEC`) now prints the storage buffers bound to the vertex and fragment stage (`vs ssbo<n> addr/size: words`).
A test that bound the whole pool behind a storage buffer (graphics stages) did not change the picture and was removed. Not run: the oracle for the decoder, the
other 138 changed shaders were not checked visually.

## Whole-game shaders — 2026-11-03 (end)

New tool: `tools/shaders/scripts/dkb_extract_bnsh.py <out> <RomFS folder or container> ...` (Python 3.14 for `compression.zstd`; no download).
It walks the RomFS (zstd files decompressed), scans for BNSH containers (variation table at BNSH+0x80, six stage descriptors per program,
control pointer +0x08, code pointer +0x10, code size +0x18, control size +0x1c, all relative to the BNSH start; layout from nxnative's notes,
the same descriptors work for NVN2) and for BAHS version 11 blocks (16-byte table entries {code offset, 0, control length, signed offset to the
control}), and writes the harness layout (`manifest.tsv`, `code_<hash>.bin`, `ctrl_<hash>.bin`, hash = SHA-1/16 of the code area).

A survey of the 77,342 RomFS files found NVN2 control sections (magic 0x98761234) in 2,500: `Effect/*.esetb.byml.zs` (31,319), `Shader/` (2,821),
`Layout/` (2,664), `Lib/agl/agl_resource...sarc.zs` (1,828; BAHS) and a handful in `Sound/Voice/Tex`. Result: **19,479 distinct stage programs**
(11,109 fragment, 8,182 vertex, 181 compute, 7 geometry) from 4,576 BNSH (47,371 variations) and 16,288 BAHS stage entries, no parse errors.
Check: 2,682 of the 2,721 shaders the running game requested at boot are in this set by hash (the format and the hashing are right);
the other 39 (29 fragment, 10 vertex) are not in any RomFS container (likely built into `main` or patched at run time; not investigated).

Translation (`spirv_harness --batch`, 87 s, `spirv-val` vulkan1.3): **19,452 ok** (compute 178, fragment 11,093, geometry 7, vertex 8,174),
27 not: 23 unsupported (7 `.UNKNOWN_000` blocks, 6 `ISETP.NE.U32.EX.AND` 64-bit compare chains, 4 `.UNKNOWN_526` (the IPA-like form seen earlier),
5 `TLD4 ... PTP`, 2 `FADD` with an odd operand, 1 `IPA.PASS` NullReferenceException, a few other unknown opcodes) and 4 exceptions (`Invalid shader stage "14"`).
110 programs have a 0xd0-byte control section (GLSLC minor 0/2) instead of 0xd8; `Program.IsNvn2` now accepts >= 0xd0 (it translated them; not checked individually).
No block uses HMMA, LDSM or MOVM (opcode scan of every program range): the NDSR upscaler is not among these containers, so that gap
is still unmeasured. As before, blocks without GLSLC reflection get guessed texture/sampler types.

Cache: `work/shaders/dkb_spirv_full/` (all `.spv` plus a merged `results.tsv`, 19,518 records, includes the 39 boot blocks from the earlier cache).
The headless probe with `NXRT_NVN_SHADER_CACHE` pointing at it ran 50 s with no errors, one block missing (the known `.UNKNOWN_526` one), menu identical.
Nothing beyond the boot scene was rendered with the other 16,000 shaders: they are translated and validated, not exercised.

## Windowed dkb.exe, input, and the GPU loss after the first Enter — 2026-11-03 (night)

`game-build/dkb` now builds `dkb.exe` (same source as `dkb_boot_probe_gfx`, `DKB_PROBE_GFX`): `dkb.exe --exefs <exefs> --window [--no-pads]`
opens an SDL window with play mode, F1 overlay and keyboard input (Enter/Space/E = A, Backspace/Shift = B, WASD = left stick, arrows = right
stick; factory bindings in `game-build/dkb/dkb_defaults.cpp`, written to `controls.ini` on first start: delete an old `controls.ini` after changing
them). Environment as for the probe (`NXRT_FS_ROMFS`, `NXRT_NVN_SHADER_CACHE`). SDL's gamepad start-up took ~30 s on this PC: `--no-pads` skips it.
Because DKB has no lifted frame-boundary site, play mode now drives its frame boundary from the SVC busy hook when the game's play table names none
(`runtime/src/gfx/play/play.cpp`, `g_self_driven`); this also keeps the window responding while the game loads.
Also: `ISelfController` cmd 19 (`SetScreenShotImageOrientation`, per Switchbrew) answered with success.

What happened with real input: Enter on the "Normal Mode" menu leads the game on (it reads `SaveMenu`/`DeleteMenu` layouts and `config.sav`, then loads a scene,
~48k texture ids). **Then the GPU is lost** (`vkQueueSubmit2 failed (-4)`, 1-10 s after the first Enter, every run); Windows logs `nvlddmkm` event 153 each time.
Not a TDR-style timeout message, not out of memory (nvidia-smi: ~2.7 of 16 GB used).

Checked and ruled out: the 21 compute shaders enabled by the 32-slot patch (still lost without them); Vulkan validation (nothing reported before the loss; the
early `vkCmdCopyImage` offset.z errors at 3.5 s are an unrelated runtime issue worth fixing); VRAM exhaustion; `[gfx] robust_buffer_access` (new option, default off, kept
the device lost). Seven fragment shaders were in the submit that failed (2f51d17a, bde9711a, 46af1ba6, ...): skipping them did not help, but the ring (below) notes draws
before the skip, so that test is not conclusive either.

Open lead, unproven: the game calls the NVN2 procs `nvnCommandBufferPushGraphicsConstants` (cmd, 0, 0, 0x10, data = 1/w, 1/h, w, h) and
`PushComputeConstants` (cmd, 0, 0, 0x20, data) right before the scene; they are logging stubs. No shader reads an extra bank for them, so where the
data should land is unknown. Not implemented. Also still stubs: `SetComputeOverlapMode`, `SetGeometryProcessingMode`, mesh-task and ray-tracing procs,
GPU-side WaitEvent.

New debug aids: `[nvn] sync_submits = true` (`NXRT_NVN_SYNC_SUBMITS=1`) waits after every submit and, if the GPU failed, logs the draws/dispatches that submit recorded
(`recent_ops_text`); the draw trace prints storage buffers (`NXRT_NVN_TRACE_SEC`). Known gap: the ring notes a draw before `skip_fs_file` is applied.

## GPU loss hunt, part 2 — 2026-11-03 (late night)

Reference read (read-only, public repo https://github.com/Lucas-blip-png/dkb-pc-recomp, GPL-3.0, README and `docs/devlog.md`): another DKB port reports the same
NVIDIA fault family ("invalid instruction pointer", instanced light-quad draws, mitigation: skip those draws), and that NVN2 push constants land in root tables
(graphics: bank 24, read only by full-screen blit shaders; compute: `c[0]` after a 16-byte head, eight root tables with word counts at control `+0xa9..+0xb0`).
Checked against our data: no graphics shader of the 19,298 extracted reads a bank above 0x10, and no compute shader declares root tables, so the unimplemented
`PushGraphicsConstants`/`PushComputeConstants` are **not** shown to be the cause here. Not implemented.

Fixed (real bug, validation-confirmed): `copy_texture_to_texture` passed the region's z as `offset.z` for 2D/array/cube images (the z is a layer index); it is now
`baseArrayLayer` (`vkCmdCopyImage` and the blit path, `nvn_replay.cpp`). The `VUID-vkCmdCopyImage-*-01787/01788` errors are gone.

Ruled out: instanced draws (skipping all of them, `NXRT_NVN_SKIP_INSTANCED_DRAWS`), light-list readers (`NXRT_NVN_SKIP_LIGHT_SSBO_DRAWS`), robustBufferAccess, VRAM at the first loss,
Vulkan validation before the loss. **The culprit moves** when a draw is skipped: with `NXRT_GFX_DIAGNOSTIC_CHECKPOINTS=1` (VK_NV_device_diagnostic_checkpoints, new) the lost
device reported `draw vs 48f02544 fs 45ab0a1f`, then after skipping `vs 48f02544` a 1x1x1 `dispatch cs ec64db4e` much later (marker 829,836 vs 114,622), and
before that `vs c2b56dc5 / 0ee41791 / 6a810154`. That pattern points at something not tied to one shader: a GPU access to memory that becomes invalid (the guest
memory pools are imported into Vulkan; a freed or remapped range would fault the GPU) or, late in a run, device memory exhaustion (`vmaCreateImage failed (-2)` precedes the loss at ~91 s
in the longest run). Neither is confirmed.

With the workaround skip list the run lived ~90 s in the scene (20-30 fps, audio on) and the window showed a flat light-purple frame; the user saw the same
purple screen (its colour is near the (0.5, 0.5, 1.0) default of the normal target, i.e. the final image is not being composed yet).

Tools added this round: `NXRT_GFX_DIAGNOSTIC_CHECKPOINTS=1`, `NXRT_NVN_SYNC_SUBMITS=1`, `NXRT_NVN_SKIP_FS_FILE` (now matches vertex shader hashes too and logs after skipping),
`NXRT_NVN_SKIP_LIGHT_SSBO_DRAWS`, `NXRT_NVN_SKIP_INSTANCED_DRAWS`, `NXRT_GFX_ROBUST_BUFFER_ACCESS`; `game-build/dkb/run_dkb.ps1` starts the window build with the right environment.

## GPU loss and VRAM growth, resolved for the first minutes — 2026-11-03 (night)

Two independent problems; both were measured with `NXRT_DIAG_SOAK_CSV` (the soak sampler, now on in `run_dkb.ps1`; gauges `vma.*`, `vk.image_bytes`, `vk.buffer_bytes`).

1. **VRAM growth (fixed).** DKB places a new 2048x2048x2 DEPTH16 texture in the virtual pool about every five texture ids. `texture_registry_find_alias` returned early
   for textures without host storage (`mem == null`), so every one got its own 16 MB image: 15 GB in use and 28 GB allocated after ~90 s, then `vmaCreateImage -2` and a lost
   device. Textures in the same virtual pool at the same offset and shape now share one image (`nvn_texture_registry.cpp`); VRAM settles at ~2-3 GB.
   Debug lines: `image '...'` (images of 8 MB and more, first 300) and `big texture N` (first 120).
2. **GPU loss (worked around).** Bisecting with `NXRT_NVN_SKIP_FS_FILE` (now also matches geometry shader hashes): with only geometry shader `498d2702dda52d98` drawing the
   device is lost at ~39 s with ~2 GB in use; with it skipped the scene runs 90-100 s (the other three storage-writing geometry shaders are not the cause).
   `[nvn] skip_gs_hashes` (`NXRT_NVN_SKIP_GS_HASHES`) drops draws using it; `run_dkb.ps1` defaults it to `498d2702`. The reason it faults is not known; the final image is
   still the flat purple frame (not composed), so the skipped draws' effect on the picture is unmeasured.

Reference (read-only): EGLANTINE `DKB_WINDOWS_PROGRESS.md` and `NX2_NOTES.md` describe the same purple screen (a terrain culling geometry shader whose list writes never reach
the second pass, replaced there by a CPU stand-in; and a tensor upscaler, HMMA.16816.F16).

## Purple screen diagnosed: the upscale pass has no shader — 2026-11-03 (night, part 2)

Trace of one scene frame (`NXRT_NVN_TRACE_FRAME=9800`, with `skip_gs_hashes` active, so the device lives): the scene is drawn correctly into `t27063` (R11G11B10F 1920x1080,
mean ~0.23/0.13/0.20) and post-processed at 1440x810, 960x540 and lower. The last pass before the present is `dispatch 60 34 1 ... cs -`: a compute dispatch with
**no program bound** (60x34 groups of 32 = 1920x1080). It should write the 2880x1620 image `t1387` (cleared to 0.3/0.3/1 at the start of the frame); the final draw into the
swap image `t3` samples `t1387`, so the window shows that clear colour (the purple screen). It is the upscaler pass, not the terrain.

Why it has no program: the game builds some programs through the NVN2 `nvnProgramBuilder*` API (`SetDefaults`, `SetDevice`, `SetShaders(builder, count, entries)`,
`nvnProgramInitializeFromBuilder(program, builder)`), which were logging stubs. They are now implemented in `api_program.cpp` (the entries are assumed to have the 0x18-byte
stride of `nvnProgramSetShadersExt`; unverified). With that, the builder programs reach `SetShaders` and the log lists **28 blocks with no translated shader** (e.g.
`cf4c5636`, `aaaba2b9`, `3decfd15`, `8569d67d`, `a9863b38`, `e2f8d329`, `632c8af8`, `cbe6ab44`, `635823b1`, `5a8d2482`, code sizes 0x300-0xa400), plus one program with a stage
that has no code or control address. They are not in the RomFS containers extracted by `dkb_extract_bnsh.py` (the 39 boot blocks that were already missing: they are built into
`main` or patched at run time). Next steps:

1. Find those blocks inside `main` (scan its data for NVN2 control sections, magic `0x98761234`, and the code they point to), extend `dkb_extract_bnsh.py` or add a second
   extractor, translate them with `spirv_harness` and add them to `work/shaders/dkb_spirv_full/`.
2. Check whether the upscaler (NDSR, tensor cores: HMMA.16816.F16, LDSM, MOVM) is among them; the Ampere frontend has no implementation of those opcodes yet. EGLANTINE
   (`NX2_NOTES.md`, `docs/dkb-render-investigation.md`) has a software HMMA for reference (GPL-3.0, read only).
3. Verify the builder entry stride (log the entries once) and the program with the stage that has no addresses.

Not yet re-checked after the builder change: whether the traced frame still ends in an empty dispatch. dkb.exe was stopped at ~47 s of that run.

## The missing blocks are agl's NDSR upscaler pack — 2026-11-03 (night, part 3)

- **Builder stride corrected.** nxnative (MIT, `dm/gfx/nvn/hle_nvn.c`) reads `nvnProgramBuilderSetShaders` entries as plain 16-byte `NVNshaderData`, not the 0x18-byte Ext
  entries assumed before; `api_program.cpp` now copies `NVNshaderData[count]` and calls `nvnProgramSetShaders`.
- **NDSR extraction.** `dkb_extract_bnsh.py` now also reads `NDSR` packs (`agl_ndsr.pck` inside `Lib/agl/agl_resource...sarc.zs`; 16-byte records from +0x10 or +0x14,
  layout from nxnative's `shader_split.py`, read not copied). It found 30 NDSR stages. Run it with Python 3.14 on the folder `romfs/Lib/agl` (a single zstd file argument is not decompressed).
  Output: `work/shaders/dkb_ndsr_all` (everything) and `work/shaders/dkb_ndsr` (54 blocks not yet in `dkb_spirv_full`).
- **Check.** After the rebuild, a 60 s headless run lists **31 distinct blocks with no translated shader; all 31 are in the NDSR extract** (24 compute, 4 fragment, 3 vertex).
- **Translation of those 54 blocks** (`spirv_harness --batch`, needs `DOTNET_ROOT=build/toolchain/dotnet`): 1 ok, 47 unsupported, 6 exceptions. All 24 upscaler compute shaders stop on
  `.UNKNOWN_23c` (HMMA), `.UNKNOWN_23a` (MOVM) and `.UNKNOWN_83b` (LDSM); the other failures are the known `ISETP.NE.U32.EX.AND` 64-bit chains, TLD4 `.PTP`, stage-14 and
  `GetConstantBufferBinding` index exceptions.
- **Next:** (a) implement HMMA.16816 / LDSM / MOVM in `ampere/decoder` + `AmpereEmit.cs` (per-lane subgroup shuffles in the PTX `mma.m16n8k16` / `ldmatrix` / `movmatrix`
  layouts; nxnative warns that LDSM's mode bits are not a stride); or (b) first a labelled stand-in that blits the composite into the 2880x1620 target when the NDSR dispatch has no
  program (EGLANTINE does the same). Then the vertex/fragment failures above.

## NDSR stand-in: the first real DKB picture — 2026-11-03 (night, part 4)

`do_dispatch` (`nvn_replay.cpp`) now answers a **compute dispatch with no program** with `ndsr_standin()` (`[nvn] ndsr_standin`, default on; **STAND-IN, not the game's upscaler**):
a bilinear `vkCmdBlitImage` from the newest R11G11B10F colour target the frame drew to (`g_recent_rt`, filled in `begin_draw`; width at most x*32 and above half of it, smaller than the
destination) into the whole storage image bound at compute image slot 0..15 (`t1387`, 2880x1620). The dispatch covers the *input* (60x34 groups = 1920 wide; 72x41 in
other frames), not the output. Not every program-less dispatch is the upscaler; the shape test is the only guard.

Result (`DKB_SHOTS=45,60,80`, windowed, 95 s, no GPU loss): the window shows the real scene (the mining-tunnel entrance with the monkeys, mine cart, red girders), not the purple frame.
Sharpness is that of a bilinear upscale. Not checked: colours against a reference, the other scenes, the menu, long runs. `run_dkb.ps1` takes `DKB_SHOTS` (seconds) for screenshots
(`build/dkb-probe/dkb_shot_N.png`); the info log is `build/dkb-probe/dkb_boot_probe.log`.
Next: HMMA/LDSM/MOVM in the Ampere frontend to replace the stand-in; the 4 fragment and 3 vertex NDSR blocks; `skip_gs_hashes` still hides terrain culling draws.

## MILESTONE: working picture (tag `dkb-first-picture`) — 2026-11-03

First commit in which dkb.exe shows the real game scene in the window (mining-tunnel entrance, not the purple frame), with no GPU loss in a ~95 s windowed run.
To return to it or copy the recipe: `git checkout dkb-first-picture`, build as above, then
`$env:DKB_SHOTS='45,60,80'; game-build/dkb/run_dkb.ps1 -Dump '<dump>/updates/458752'` (screenshots in `build/dkb-probe/dkb_shot_N.png`).

What makes the picture work (all four are needed):
1. Texture alias sharing for virtual-pool textures (`nvn_texture_registry.cpp`): stops the VRAM growth that lost the device.
2. `[nvn] skip_gs_hashes` default `498d2702` (`run_dkb.ps1`): workaround for the GPU fault; terrain culling draws are therefore missing.
3. NVN2 program builder (`api_program.cpp`, plain 16-byte `NVNshaderData` entries) and NDSR extraction (`dkb_extract_bnsh.py`).
4. NDSR **stand-in** (`ndsr_standin()` in `nvn_replay.cpp`): bilinear blit instead of agl's tensor-core upscaler. Replace it when HMMA/LDSM/MOVM are translated.

Known gaps at this milestone: upscaler is a stand-in; geometry shader `498d2702` draws skipped; 4 fragment and 3 vertex NDSR blocks untranslated; colours, other scenes and long runs unchecked.

## HMMA / LDSM / MOVM translated, but the live NDSR shaders lose the GPU — 2026-11-03 (night, part 5)

- **Translator.** `ampere/decoder/SassDecoder.Tensor.cs` decodes HMMA (0x23c: .1688 / .16816, F16 or F32 accumulator), LDSM (0x83b: 1/2/4 matrices, M88 or transposed MT88) and MOVM (0x23a, MT88);
  field positions from NAK `sm70_encode.rs` (MIT; fetched to the ignored `work/nak/`). `AmpereEmit.cs` (`EmitHmmaAmpere`, `EmitLdsmAmpere`, `EmitMovmAmpere`) emulates them per lane with
  subgroup shuffles in the PTX mma.m16n8k16 / ldmatrix / movmatrix fragment layouts (32 shuffles and 64 FMAs per HMMA.16816; F16 accumulate is done in f32 and rounded once at the end).
  Result: **24 of the 27 NDSR compute shaders translate and pass `spirv-val`** (SPIR-V of 5-6 MB each); 3 remain (SUATOM `.UNKNOWN_593` x2, `.UNKNOWN_481`). They are merged into
  `work/shaders/dkb_spirv_full` (`results.tsv.bak_pre_ndsr` is the previous table).
- **Not verified numerically.** There is no GPU or CPU reference test for the three ops yet; the layouts are from the PTX documentation, not checked against hardware output.
- **Live result: device lost.** With the NDSR compute shaders active the game loses the GPU after ~53 s (`vkQueueSubmit2 failed (-4)`, window black). `NXRT_NVN_SKIP_CS_HASHES` (new,
  `[nvn] skip_cs_hashes`, hash prefixes; a skipped compute program takes the stand-in path) bisected it: skipping all 27 is stable; `efc55988` alone loses the device, and so does
  everything except `efc55988` (so more than one shader does, probably all the large ones). Cause unknown: candidates are the driver/TDR on 5-6 MB shaders, the c[0] push-constant /
  storage layout of NVN2 compute (nxnative: "compute lays out c[0] per shader from its control section, push constants first"), or a bug in the emulation.
- **Default.** `run_dkb.ps1` sets `NXRT_NVN_SKIP_CS_HASHES` to all 27 NDSR prefixes, so the default run is the stable stand-in picture. Set it to another value (e.g. `none`) to try the real shaders.
- **Next:** run one NDSR shader in isolation (a small compute test with known input) to check the emulation, check the compute c[0] layout for these shaders, then compile-time (2 s for the 5 MB SPIR-V
  at pipeline creation) and TDR behaviour.
