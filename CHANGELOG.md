# Changelog

All notable changes to LAST CALL, in both languages. Sections are headed with the exact version from [VERSIONING.md](VERSIONING.md); the newest section is at the top. The Russian twin of every entry lives in [CHANGELOG.ru.md](CHANGELOG.ru.md).

## Unreleased

- F0 in progress: UE 5.8 project skeleton (`LastCall.uproject`, `Source/` runtime + editor modules, phase clock `ULCPhaseManager`, noir renderer/input configs), design data with JSON Schemas, and three linters — `lc-data-lint`, `lc-dialogue-lint` (graph + EN/RU parity), `lc-docs-check` (twin links + link integrity) — running locally and in CI. Open: engine compile on Linux/Windows (no UE on the machine yet).
- Documentation set created: README, PHILOSOPHY, ARCHITECTURE, ROADMAP, CHANGELOG, CONTRIBUTING, CODE_OF_CONDUCT, SECURITY, VERSIONING — each in English and Russian, linked pairwise.
- Game documents: [GAME-DESIGN](docs/GAME-DESIGN.md), [NARRATIVE](docs/NARRATIVE.md), [ART-BIBLE](docs/ART-BIBLE.md), [LANGUAGES](docs/LANGUAGES.md), [FREE-3D-MODELS](docs/FREE-3D-MODELS.md), [MAINTENANCE](docs/MAINTENANCE.md).
- Stack decided: Unreal Engine 5 (5.8) + C++ for the game, Rust (Zig acceptable) for pipeline tools, Python for editor scripting (owner decision, 2026-10-13, with cited research in [docs/LANGUAGES.md](docs/LANGUAGES.md)).
- Free 3D assets catalog: ~90 assets with URLs fetched and licences recorded (CC0 / CC-BY / platform EULA), plus a grey-box baseline set (verified 2026-10-13).
- License decided: **MIT** for code and documentation, game content stays proprietary ([LICENSE](LICENSE), owner decision 2026-10-13).
- Game text decided: written natively in **both** EN and RU from the start, with a parity lint over `Data/Localization/` (owner decision 2026-10-13).
- Team shape recorded: solo developer + AI agents; milestones are worked sequentially and autonomously.
- Documentation maintenance routine defined: bilingual sync, review at every session, change log discipline (see [docs/MAINTENANCE.md](docs/MAINTENANCE.md)).
