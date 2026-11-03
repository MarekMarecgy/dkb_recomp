# DKB Windows bring-up — 2026-11-03

Latest executable milestone and resume instructions: **[BUILD.md](BUILD.md)**.

## Current base

The user requested replacing the old folder with Pokopia Recomp and updated DKB
Markdown files. The source-folder migration is complete:

- Active folder: `C:/Users/Marek/Desktop/DKBRecomp`.
- Active source: Pokopia Recomp / nxrecomp at `2c29515b23a6d78d967775fd7e9f24f50402cca7`.
- Local branch: `codex/dkb-prototype`; remote `upstream` points to Pokopia Recomp.
- Licence: upstream GPL-3.0 and the retained third-party notices.
- Old folder: `C:/Users/Marek/Desktop/DKBRecomp-backup-totk-20261103`.
- The source base now also includes the DKB recompiler profile, native diagnostic
  checker and build portability/headless-link fixes listed in BUILD.md.
- No commits, pushes or PRs were created. EGLANTINE was not modified.

The source replacement does not make DKB playable. Pokopia's `pkp` profiles, hooks
and executable/launcher branding remain. The DKB recompiler profile and three module
libraries exist, but the actual DKB runtime startup is not implemented.

## Inputs and observed CPU coverage

Existing dump: `C:/Users/Marek/Desktop/EGLANTINE/GAMES/DKB ROM/`.
The evaluated update is `458752`, with modules `main` and `sdk`, title ID
`040074A01BF12000`. Read it in place; do not change the dump or saves.

Before migration, this exact Pokopia revision's `nxrecomp cover` command ran on DKB:

| Module | Accepted instruction words | Rejected |
|---|---:|---:|
| main | 7,207,192 | 0 |
| sdk | 2,021,736 | 0 |

Logs remain in the backup's `build/pokopia-evaluation/`. The Pokopia recompiler has not
changed since that check. `cover` also accepts deliberate traps/padding and the inherited
unsigned-PAC policy; this does not prove semantic equivalence, complete compilation,
function-discovery correctness, SDK boot or graphics correctness.

The former TotK prototype's 274 compiled C translation units and native tests belong
to the backup. Those results do not validate this C++/nxrecomp runtime.

## Next milestones

1. Extend the existing recompiler profile with DKB runtime integration under
   `runtime/games/dkb/`. Do not mix the former prototype's context or objects in.
2. Establish DKB's SDK startup, constructors and safe interception sites. Wire a DKB
   executable without Pokopia-specific hooks. Verify memory, clock, thread and graphics
   initialization behavior with a boot probe.
3. The three DKB libraries compile and register successfully; next validate actual
   guest execution and investigate CPU/ABI gaps exposed by SDK startup.
4. Adapt shader extraction: DKB uses BNSH/BAHS/NDSR, unlike Pokopia's KTGL RDB/G2S.
   Use the established nxnative DKB findings as reference.
5. Run the DKB shader corpus through the Ampere frontend. The inspected decoder/frontend
   had no HMMA/LDSM/MOVM implementations; DKB's NDSR upscaler uses these. Measure the
   gaps and implement or deliberately replace the required path.
6. Validate terrain geometry/culling, indirect records, conditional clears and texture
   layout. Reach the first correct scene before polishing launcher branding or UI.
7. Test guide movies, saves, graphics modes and longer gameplay before making
   compatibility, stability or frame-rate claims.

See [the evaluation](pokopia-evaluation.md) for source references. The nxnative checkout
remains in the backup's `refs/nxnative/`.

## Build notes

Use the current [recompiler README](../recomp/README.md),
[runtime README](../runtime/README.md) and [shader instructions](../tools/shaders/README.md).
The old `tools/dkb.py`, `recomp/gen.py`, root `.venv` and `config.local.toml` were not
carried into this folder. A new `tools/.venv` and a project-local LLVM 23.1.4 toolchain
are now installed; BUILD.md records their paths and the successful build commands.

Upstream documents Python 3.13. The coverage evaluation used Python 3.12 successfully;
that does not establish compatibility with the entire build. Check project files for
.NET requirements: the Ampere decoder targets .NET 10.

All game inputs, keys, saves, generated code and build outputs must stay ignored by Git.
