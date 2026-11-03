# Source base and attribution

The active base is [drakolordx7/pokopia-recomp](https://github.com/drakolordx7/pokopia-recomp)
at `2c29515b23a6d78d967775fd7e9f24f50402cca7`, built on
[nxrecomp](https://github.com/drakolordx7/nxrecomp). The root [LICENSE](LICENSE) declares
GPL-3.0. Individual files and dependencies retain their original notices.

Preserve the upstream attribution records:

- [Runtime third-party notices](runtime/THIRD_PARTY_NOTICES.md).
- [Recompiler third-party notices](recomp/THIRD_PARTY.md).
- [Shader provenance](tools/shaders/README.md) and [licence files](tools/shaders/licenses/).
- [Texture-tool third-party notices](tools/textures/THIRD_PARTY.md).

DKB findings also reference [seedds/nxnative](https://github.com/seedds/nxnative) at
`7ef2aaf54ad63747623ae044f2963e310a031f11`, particularly
`dm/games/040074A01BF12000/README.md` and `game.tsv`. Its host/graphics implementation
has not been integrated during this migration. Add attribution when code is reused.

The former TotK prototype and its notices remain in the separate backup. Its
implementation and generated output are not part of this source tree.
