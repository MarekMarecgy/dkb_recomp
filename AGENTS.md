# DKBRecomp

This is Pokopia Recomp / nxrecomp being adapted to Donkey Kong Bananza on Windows.
Read [README.md](README.md), **[docs/BUILD.md](docs/BUILD.md)**, [docs/dkb-bringup.md](docs/dkb-bringup.md) and
[docs/pokopia-evaluation.md](docs/pokopia-evaluation.md) before continuing.

## Work boundaries

- Never submit PRs or push commits on behalf of the user. Do not create commits unless requested.
- Work in `C:/Users/Marek/Desktop/DKBRecomp`. Keep EGLANTINE and the old backup unchanged.
- The base is Pokopia Recomp at `2c29515b23a6d78d967775fd7e9f24f50402cca7`, not TotK.
  Do not restore the former MIT-only reuse rule or TotK tooling here.
- Preserve the GPL licence and existing SPDX, copyright and third-party notices.
  Record the source and licence of reused code.
- Keep comments concise and relevant. Documentation, code and comments are in English.
- Treat Pokopia's offsets, hooks, shader keys, save paths and branding as game-specific.
  Do not apply them to DKB merely because both games target Switch 2.

## Inputs and outputs

- The local dump remains at `C:/Users/Marek/Desktop/EGLANTINE/GAMES/DKB ROM/`.
  The evaluated update is `updates/458752/exefs`, modules `main` and `sdk`.
- Read game inputs in place. Do not alter the dump, keys or user saves.
- Keep binaries, generated source, extracted resources, logs and other game-derived files
  in ignored `work/`, `build/` or `game/` directories. Never commit game content or keys.
- Keep local configuration paths ignored. The old `config.local.toml` and `tools/dkb.py`
  belong to the backup and are not tools for this base.
- Backup: `C:/Users/Marek/Desktop/DKBRecomp-backup-totk-20261103`. Its C code, CPU
  contexts and objects use a different ABI and must not be linked into this runtime.

## Implementation and validation

- The recompiler profile is in `recomp/games/dkb/`; the native diagnostic checker is
  in `game-build/dkb/`. The actual `runtime/games/dkb/` integration remains to be added.
- DKB module builds require LLVM 23 or newer; LLVM 19 crashed compiling this input.
  `runtime/scripts/env.cmd` prefers the local verified LLVM 23.1.4 toolchain.
- Follow `recomp/README.md`, `runtime/README.md` and the relevant CMake files/scripts.
  Generated code uses C++ and clang-cl, not the former C generator.
- Change generators rather than editing output. Run appropriate differential/oracle
  checks after lifter changes and relevant runtime/shader checks for affected subsystems.
- `cover` acceptance does not prove execution. Distinguish decoder coverage, native
  compilation, SDK boot, rendering and gameplay checks. Keep unsupported cases explicit.
- The inherited PAC policy uses unsigned pointers, not cryptographic authentication
  emulation. Account for the limitations in `recomp/docs/GAPS.md` when validating DKB.
- Verify DKB's shader/container, upscaler and terrain requirements with evidence.
  An Ampere frontend alone does not establish complete DKB shader support.
- Do not report the old TotK prototype's tests as validation of this runtime.
