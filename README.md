<div align="center">

# LAST CALL

**A story-driven bar-management sim with action and RPG elements.** You are Alex, a bartender who inherited an old bar in the center of a metropolis — neutral territory for three criminal gangs and a secret communication hub.

[![Status](https://img.shields.io/badge/status-pre--production-orange)](ROADMAP.md)
[![Platform](https://img.shields.io/badge/platform-PC-89b4fa.svg)](ROADMAP.md)
[![Engine](https://img.shields.io/badge/engine-Unreal%20Engine%205-e05656.svg)](ARCHITECTURE.md)
[![Docs](https://img.shields.io/badge/docs-EN%20%7C%20%D0%A0%D1%83%D1%81-blue.svg)](docs/MAINTENANCE.md)
[![License](https://img.shields.io/badge/license-MIT%20(code)%20%7C%20content%20proprietary-blue.svg)](LICENSE)

</div>

> **Status:** Pre-production. The concept, the design philosophy and the roadmap are written; the engine prototype has not started. Documentation is bilingual — every document exists in English and Russian, linked pairwise at the top of each file — and is kept current by a [regular review routine](docs/MAINTENANCE.md). See the [changelog](CHANGELOG.md) and the [roadmap](ROADMAP.md).

**[Русская версия этого файла →](README.ru.md)**

---

## The concept

Your bar is the city's **Swiss army knife**. Office workers fill it by day, criminals by night, and downstairs — in a secret bunker with illegal equipment — the real job begins after closing time.

The bar is not just a place to drink. It is neutral ground for three gangs *and* a hidden communication node. Your uncle did not just die: he was the keeper of information that can destroy the city — information **encrypted in the cocktail recipes on the wall**, wanted by everyone.

**Formula:** *Papers, Please* meets *Grand Theft Auto* in the décor of a chaotic wedding comedy. You balance the art of mixology, the criminal world and running a business. The strongest skill is knowing **when to listen and when to pour**.

## The day, in three phases

| Phase | When | Gameplay |
| :--- | :--- | :--- |
| **1. Prep** | Morning | Buying supplies, ice, alcohol. Upgrading the bar (taps, mixer, VIP zone). Crafting your own infusions and syrups — experiment until a signature cocktail becomes the bar's calling card. |
| **2. Shift** | Day / evening | Liquid physics pouring (tilt the bottle, control the stream), customers with mood and patience meters, listening to their stories, reading body language, and disarming a fight before it starts. |
| **3. Night** | After closing | Action / stealth: take-away orders through the armored window, collecting debts with bar tools as weapons, investigating with overheard conversations. |

## Key mechanics

- **Cocktail-combo system** — pour in the right sequence at the right speed for a critical success: the customer gets a mood buff, you get better tips and XP.
- **Bartender skill tree** — Flair (juggling bottles), Psychology (see hidden desires), Steady hands (less aim shake during a shootout), Alchemy (special ingredients).
- **Dynamic events** — the police can raid the place; hide the illegal alcohol or distract them while an NPC partner stashes the evidence.
- **Destructibility** — a broken bottle, a flipped table, a soda siphon used as a fire extinguisher.
- **Co-op for 2–4 players** — bartender, waiter, bouncer, DJ; coordinate or drown in the crowd.
- **Bar progression** — from a dirty dive to a luxury club with several floors and its own underground casino.

Full design: [docs/GAME-DESIGN.md](docs/GAME-DESIGN.md) · Story and characters: [docs/NARRATIVE.md](docs/NARRATIVE.md) · Look and sound: [docs/ART-BIBLE.md](docs/ART-BIBLE.md)

---

## Technology

| Layer | Decision |
| :--- | :--- |
| Engine | Unreal Engine 5 (Lumen GI, Nanite, Chaos) — [why](docs/LANGUAGES.md) |
| Gameplay code | C++ (UE5 modules), Blueprints for iteration |
| Liquid physics | Custom pour simulation (see [ARCHITECTURE.md](ARCHITECTURE.md)) |
| Tools / pipeline | Rust (or Zig) CLI tools for asset cooking, dialogue linting, localization checks |
| Scripting / data | UE DataTables, JSON/YAML sources under version control |
| Multiplayer | UE replication, listen-server co-op for 2–4 players |
| Game text | Written natively in **both** EN and RU from the start (`Data/Localization/`) |
| Documentation | Markdown, bilingual EN/RU, reviewed on the routine in [docs/MAINTENANCE.md](docs/MAINTENANCE.md) |
| Free 3D assets | Cataloged with licenses in [docs/FREE-3D-MODELS.md](docs/FREE-3D-MODELS.md) |

Language and engine comparison (C++, Rust, Zig, Odin, Godot, Bevy): **[docs/LANGUAGES.md](docs/LANGUAGES.md)**

## Documentation map

| Document | English | Русский | Purpose |
| :--- | :--- | :--- | :--- |
| README | [README.md](README.md) | [README.ru.md](README.ru.md) | Entry point |
| Philosophy | [PHILOSOPHY.md](PHILOSOPHY.md) | [PHILOSOPHY.ru.md](PHILOSOPHY.ru.md) | What the game is and why; wins over every other document |
| Architecture | [ARCHITECTURE.md](ARCHITECTURE.md) | [ARCHITECTURE.ru.md](ARCHITECTURE.ru.md) | How it is built |
| Roadmap | [ROADMAP.md](ROADMAP.md) | [ROADMAP.ru.md](ROADMAP.ru.md) | In what order |
| Changelog | [CHANGELOG.md](CHANGELOG.md) | [CHANGELOG.ru.md](CHANGELOG.ru.md) | What changed, build by build |
| Contributing | [CONTRIBUTING.md](CONTRIBUTING.md) | [CONTRIBUTING.ru.md](CONTRIBUTING.ru.md) | Workflow |
| Code of Conduct | [CODE_OF_CONDUCT.md](CODE_OF_CONDUCT.md) | [CODE_OF_CONDUCT.ru.md](CODE_OF_CONDUCT.ru.md) | Behaviour |
| Security | [SECURITY.md](SECURITY.md) | [SECURITY.ru.md](SECURITY.ru.md) | How to report vulnerabilities |
| Versioning | [VERSIONING.md](VERSIONING.md) | [VERSIONING.ru.md](VERSIONING.ru.md) | Build and release scheme |
| Game design | [docs/GAME-DESIGN.md](docs/GAME-DESIGN.md) | [docs/GAME-DESIGN.ru.md](docs/GAME-DESIGN.ru.md) | The GDD |
| Narrative | [docs/NARRATIVE.md](docs/NARRATIVE.md) | [docs/NARRATIVE.ru.md](docs/NARRATIVE.ru.md) | Story, characters, factions |
| Art bible | [docs/ART-BIBLE.md](docs/ART-BIBLE.md) | [docs/ART-BIBLE.ru.md](docs/ART-BIBLE.ru.md) | Noir look, light, sound |
| Languages | [docs/LANGUAGES.md](docs/LANGUAGES.md) | [docs/LANGUAGES.ru.md](docs/LANGUAGES.ru.md) | C++, Rust and friends |
| Free 3D models | [docs/FREE-3D-MODELS.md](docs/FREE-3D-MODELS.md) | [docs/FREE-3D-MODELS.ru.md](docs/FREE-3D-MODELS.ru.md) | Assets we may use, with licenses |
| Doc routine | [docs/MAINTENANCE.md](docs/MAINTENANCE.md) | [docs/MAINTENANCE.ru.md](docs/MAINTENANCE.ru.md) | How and when docs are updated |

## Repository layout

```
README.md / README.ru.md     entry point, bilingual pair
PHILOSOPHY.md                design principles (highest authority)
ARCHITECTURE.md              technical design
ROADMAP.md                   milestones with status markers
CHANGELOG.md                 build history
CONTRIBUTING.md              how to take part
CODE_OF_CONDUCT.md           behaviour rules
SECURITY.md                  vulnerability reporting
VERSIONING.md                version format and release flow
docs/                        GDD, narrative, art, languages, assets, doc routine
```

## Contributing

1. Read [PHILOSOPHY.md](PHILOSOPHY.md) — a feature that goes against it will be declined, however well it is built.
2. Read [CONTRIBUTING.md](CONTRIBUTING.md) for the workflow.
3. Pick something from [ROADMAP.md](ROADMAP.md) or open an issue.

Every document exists in both languages; changing one means updating its twin (see [docs/MAINTENANCE.md](docs/MAINTENANCE.md)).

## License

**[MIT](LICENSE) for code and documentation.** The game content — art, audio, story, dialogue and proprietary assets — is **not** covered by MIT and remains proprietary. Free third-party assets keep their own licenses, recorded per asset in [docs/FREE-3D-MODELS.md](docs/FREE-3D-MODELS.md). Owner decision, 2026-10-13 (see [PHILOSOPHY.md](PHILOSOPHY.md) §4).
