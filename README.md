# DKBRecomp

Donkey Kong Bananza Windows port in progress, now based on
[Pokopia Recomp](https://github.com/drakolordx7/pokopia-recomp) and nxrecomp.
The source base has been replaced; the code still contains Pokopia's game profile,
hooks, launcher and branding. **DKB does not boot through this project yet.**

Upstream base: `2c29515b23a6d78d967775fd7e9f24f50402cca7`.
Local branch: `codex/dkb-prototype`. No commits or pushes were made for the migration.

## Current state

- The Pokopia/nxrecomp recompiler, Switch 2 runtime and Ampere-to-SPIR-V tools are the active base.
- DKB update 458752 instruction coverage was checked on `main` and `sdk`: both commands
  accepted every instruction word. This is translation acceptance, not game execution.
- The DKB recompiler profile and module libraries now build with LLVM 23.1.4. A native
  checker registers 190,997 functions; SDK startup and DKB rendering remain to be implemented.
- The graphics demo runs on the RTX 5070 Ti without Vulkan validation errors. The inherited
  core suite has unresolved failures (62/73 groups passed).
- The former TotK prototype, tools, environment and results are preserved separately in
  `../DKBRecomp-backup-totk-20261103/`.

For resuming work, start with **[the build results and handoff](docs/BUILD.md)**,
[the current bring-up plan](docs/dkb-bringup.md),
[the base evaluation](docs/pokopia-evaluation.md) and [AGENTS.md](AGENTS.md).

## Repository layout

| Path | Purpose |
|---|---|
| `recomp/nxrecomp/` | AArch64-to-C++ static recompiler |
| `recomp/games/dkb/` | DKB profile, input verification and module-library build |
| `recomp/games/pkp/` | Existing Pokopia profile; reference for a future DKB profile |
| `runtime/` | Horizon services, memory, threads and NVN/Vulkan runtime |
| `runtime/games/pkp/` | Existing Pokopia runtime integration |
| `tools/shaders/ampere/` | Ampere decoder and shader frontend |
| `game-build/`, `launcher/` | Existing Pokopia executable and launcher integration |
| `game-build/dkb/` | DKB diagnostic module checker; not a playable game |
| `docs/` | DKB migration findings and next milestones |

## Toolchain and checks

The upstream build documents Visual Studio 2022 Build Tools or newer, LLVM/clang-cl,
CMake, Ninja, Python 3.13, .NET, vcpkg/libvpx, Rust and Node.js. Check individual project
files: the Ampere decoder targets .NET 10 despite upstream's top-level README mentioning
.NET 8. The local build now has a Python environment and LLVM 23.1.4 under ignored
folders; see [BUILD.md](docs/BUILD.md) for validated commands and remaining dependencies.

Recompiler setup and DKB coverage check, after installing the required Python version:

```powershell
py -3.13 -m venv tools/.venv
tools/.venv/Scripts/python.exe -m pip install -r recomp/requirements.txt
$env:PYTHONPATH = "$PWD/recomp"
$env:PYTHONDONTWRITEBYTECODE = '1'
# Replace <exefs> with the existing extracted DKB update 458752 ExeFS directory.
tools/.venv/Scripts/python.exe -m nxrecomp cover '<exefs>/main' --name main
tools/.venv/Scripts/python.exe -m nxrecomp cover '<exefs>/sdk' --name sdk --base 0x7500000000
```

See [recompiler instructions](recomp/README.md), [runtime instructions](runtime/README.md)
and [shader instructions](tools/shaders/README.md). Upstream's `pkp` build commands
target Pokopia; they do not build DKB just by selecting a different dump.

## Content and attribution

Use the existing dump in place. Keep game data, keys, saves, generated code and build
results out of version control. EGLANTINE is a separate project.

This base declares [GPL-3.0](LICENSE); third-party files retain their own licences.
Preserve the [source and attribution notices](THIRD_PARTY_NOTICES.md). DKB documentation
was prepared with AI assistance. Human review and DKB gameplay validation remain pending.
No game files are included.
