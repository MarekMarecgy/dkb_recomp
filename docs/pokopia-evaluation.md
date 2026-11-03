# Pokopia Recomp evaluation and adoption — 2026-11-03

Subsequent build results, remaining test failures and resume instructions:
[BUILD.md](BUILD.md). This evaluation's coverage results remain historical evidence;
the later build compiled and registered the three actual DKB module libraries.

## Decision and migration status

Pokopia Recomp / nxrecomp is now the active source base in `DKBRecomp`, replacing the
TotK prototype at the user's request. This is a source-folder migration; DKB runtime
integration and game execution remain unfinished.

The old project is at `C:/Users/Marek/Desktop/DKBRecomp-backup-totk-20261103`, including
its reports, generated C, tests and environment. The former MIT-only reuse rule does
not apply to this new GPL source base. Preserve its licence and third-party notices.

## Evaluated source and test

Source: [drakolordx7/pokopia-recomp](https://github.com/drakolordx7/pokopia-recomp),
commit `2c29515b23a6d78d967775fd7e9f24f50402cca7`.

Its `nxrecomp cover` command accepted every word in DKB update 458752 `main` and `sdk`:

| Module | Accepted / total | Rejected |
|---|---:|---:|
| main | 7,207,192 / 7,207,192 | 0 |
| sdk | 2,021,736 / 2,021,736 | 0 |

Both exited zero. Logs remain in the backup at `build/pokopia-evaluation/`.
`cover` in `recomp/nxrecomp/program.py` checks whether lifting throws an unsupported
instruction or decoder error. It also accepts deliberate traps and padding.
Acceptance does not prove functional CPU correctness or game boot. Neither DKB nor
Pokopia has been executed through this checkout in this investigation.
Reproduction commands using the new layout are in [README.md](../README.md).

## Advantages verified in source

- `runtime/include/nxrt/game.h`: a generic game boundary with `Platform::Switch2`,
  identity, title defaults and optional game-specific tables.
- `runtime/src/core/switch2.cpp`: Switch 2 graphics boot interception, initialization
  flag/driver-pointer handling, VI support and host video-decoder integration.
- `runtime/src/gfx/nvn/api_device.cpp`: NVN2 API 231.30 for Switch 2.
- `runtime/src/kernel/time.cpp` and `runtime/src/core/abi_glue.cpp`: platform-selected
  tick rate, including system-register counter reads.
- `runtime/include/nxrt/memory.h`: a 12 GiB heap view. `runtime/src/kernel/svc_memory.cpp`
  permits Switch 2 heaps beyond the Switch 1 4 GiB limit. Actual DKB behavior still needs tests.
- `tools/shaders/ampere/decoder/` and `frontend/`: SM86 decoding and translation into
  Ryujinx IR and its SPIR-V backend for Vulkan.
- `recomp/nxrecomp/lift/tier2/x_ldst.py`: LDAPR/LRCPC handling.
- `recomp/docs/GAPS.md` and `lift/tier2/x_pac.py`: the unsigned-pointer policy. RETAA/RETAB
  lower as returns; PAC/AUT/XPAC generally leave pointers unchanged. This is not
  cryptographic authentication emulation, and upstream excludes PAC from differential
  comparisons. Its suitability for DKB remains to be tested.

## DKB-specific work

Pokopia's profile is not a DKB profile. Validate hooks, offsets, shader keys, save paths
and resource assumptions before applying them. In particular:

- DKB's BNSH/BAHS/NDSR containers need different extraction from Pokopia's KTGL RDB/G2S.
- Bounded searches of the Ampere decoder/frontend found no HMMA, LDSM or MOVM
  implementations; nxnative's DKB notes identify these in the NDSR upscaler.
- Terrain culling, indirect records, conditional clears and texture layout need DKB tests.
  Working Pokopia rendering does not validate these paths for DKB.
- Host video decoding exists, but DKB guide-movie loops need validation.

Use nxnative's DKB findings alongside this Windows base. The first runtime milestone
is DKB SDK/graphics initialization, followed by a correct first scene. The full plan
is in [dkb-bringup.md](dkb-bringup.md).
