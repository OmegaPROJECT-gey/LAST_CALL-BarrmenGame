# Contributing to LAST CALL
**[Русская версия этого файла →](CONTRIBUTING.ru.md)**

Read [PHILOSOPHY.md](PHILOSOPHY.md) first. A feature that goes against it will be declined, however well it is built. In short:

1. **Listen and pour.** Mechanics must reward attention and physical skill, not button presses.
2. **No monetisation tricks, no telemetry.** One price; single-player works offline.
3. **The pour is the game.** Any change that makes mixing less physical or less readable is not accepted.
4. **Bilingual documents.** Every document exists in English and Russian; changing one means updating its twin in the same change.

## What we need first

The project is in pre-production (see [ROADMAP.md](ROADMAP.md)). In order of usefulness:

1. **Design review** — play the concept in your head, find where the three-phase loop gets boring, and say it.
2. **Reference hunting** — bar interiors, noir lighting, liquid pouring videos, bartender technique; we collect them in issues.
3. **Free assets** — additions to [docs/FREE-3D-MODELS.md](docs/FREE-3D-MODELS.md) with license and format verified.
4. **Writing** — EN/RU dialogue drafts that fit the tone of [docs/NARRATIVE.md](docs/NARRATIVE.md).
5. **Code** — starts at milestone F0 ([ROADMAP.md](ROADMAP.md)); until then, tools in `tools/` are welcome.

## Setup

You need Unreal Engine 5.4+ (Epic Games launcher, free), Git, and a C++ toolchain the engine supports (Visual Studio 2022 on Windows, clang on Linux/macOS). Pipeline tools are written in Rust (see [docs/LANGUAGES.md](docs/LANGUAGES.md)):

```bash
git clone <this repository>
cd GameBarmenSimulator
# engine: generate project files from LastCall.uproject once it exists
cargo test --manifest-path tools/Cargo.toml   # data linters, once tools/ exists
```

Documentation changes need nothing but a text editor — that is most of the contribution until F1 lands.

## Documentation changes

- **Both languages, same change.** `FOO.md` and `FOO.ru.md` must say the same thing; links at the top of each file point to the twin.
- **Record it** in `CHANGELOG.md` (+ its twin) under `Unreleased`.
- **Hierarchy:** PHILOSOPHY wins over ARCHITECTURE, which wins over ROADMAP and the game documents. Do not change PHILOSOPHY without the owner's date-stamped decision.
- Follow the review routine in [docs/MAINTENANCE.md](docs/MAINTENANCE.md).

## Bugs and ideas

- **Bug:** search existing issues first, then open a new one: OS, build version ([VERSIONING.md](VERSIONING.md)), steps, expected/actual.
- **Feature:** agree on it in an issue before writing a large change.
- **Security:** never in issues — follow [SECURITY.md](SECURITY.md).

## Pull requests

1. Branch from `main`.
2. One logical change per pull request, linked to its issue.
3. Documentation changes: both languages, changelog entry, links checked (the repo has a link checker — see [docs/MAINTENANCE.md](docs/MAINTENANCE.md)).
4. Code changes (from F0 on): build passes on Linux and Windows, data linters pass, and a bug fix comes with a test that fails without it.
5. Working sessions — human or AI agent — never idle on background waits: while a file downloads, an archive unpacks or an engine installs, work continues on the next task (see [docs/MAINTENANCE.md](docs/MAINTENANCE.md) §6).

Everyone taking part follows the [Code of Conduct](CODE_OF_CONDUCT.md).
