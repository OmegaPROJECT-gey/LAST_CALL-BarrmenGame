# LAST CALL philosophy

This file says what LAST CALL is and why. [ARCHITECTURE.md](ARCHITECTURE.md) says how it is built, [ROADMAP.md](ROADMAP.md) in what order, [docs/GAME-DESIGN.md](docs/GAME-DESIGN.md) in what detail. Where they disagree, **this file wins**, and it wins over any agent's memory too.

Every decision below was made by the project owner on **2026-10-13** and is amended only here, with the date of the change.

## 1. What it is

A single-player story-driven bar-management sim for PC, with action and RPG layers, playable in short day-cycles. The player leads: the game listens, the player reacts.

- By default: a shift behind the bar — pour, listen, read the room.
- The night layer: action and stealth after closing time.
- The meta: a business that grows from a dive to a club, and a story that wants the recipes on your wall.

The formula in one line: **Papers, Please meets Grand Theft Auto in the décor of a chaotic wedding** — balancing mixology, crime and business management.

## 2. The three pillars

Every feature must serve at least one of these. A feature that serves none is declined.

1. **Listen and pour.** The strongest skill is attention: patience and mood meters, body language, stories that matter. Pouring is a physical act — tilt, stream, timing — not a button press.
2. **Neutral ground, thin ice.** The bar belongs to nobody and to everybody. The fun comes from keeping three factions balanced while the floor hides illegal equipment.
3. **One night, one shift.** The day is a loop of three phases (prep, shift, night) that each change the rules. Short sessions must end on a hook, not on a loading screen.

## 3. What it will not have

1. **Pay-to-win, loot boxes or any predatory monetisation.** One price, the game is the game.
2. **Always-online or telemetry of any kind.** Single-player is offline; no play data leaves the machine. If analytics are ever added, they are opt-in and count launches, never content.
3. **A minigame that is really a button press.** Mixing is fluid physics with a fail state; abstraction without tension is a rejected design.
4. **Fetched content or an asset flip.** Free assets are a starting point (catalogued in [docs/FREE-3D-MODELS.md](docs/FREE-3D-MODELS.md)); the bar itself, the main cast and the liquids are made for this game.
5. **Tone drift.** Noir with warmth, not grimdark; consequences, not torture porn. The violence is rare, sudden and expensive.

## 4. Technology

| Layer | Decision | Why |
|---|---|---|
| Engine | Unreal Engine 5 | Realism and noir lighting out of the box: Lumen GI, Nanite, Chaos; owner decision 2026-10-13 |
| Gameplay | C++ UE modules; Blueprints for iteration | Simulation logic must be testable and refactorable; UI and set-dressing iterate faster in Blueprints |
| Liquid | Custom pour simulation on top of Chaos/Niagara | No off-the-shelf bartender physics exists; see [ARCHITECTURE.md](ARCHITECTURE.md) |
| Tools & pipeline | Rust CLI (Zig acceptable) | Small, fast, safe binaries for asset cooking, dialogue linting, localization checks — without a runtime in the shipped game |
| Data | DataTables from JSON/YAML in version control | Designers edit text; the build compiles it |
| Multiplayer | UE replication, listen server, 2–4 players | Co-op is a mode, never a requirement for the campaign |
| Platform | PC first | Consoles only after the PC build is complete |
| Documentation | Markdown, EN + RU pairs, routine in [docs/MAINTENANCE.md](docs/MAINTENANCE.md) | Two languages are a feature for players and contributors, not duplication |
| Game text | Written natively in both EN and RU from the start — no translation pass later | Owner decision 2026-10-13; both languages ship (`Data/Localization/`) |
| License | **MIT** for code and documentation; game content and proprietary assets stay proprietary; free assets keep their own licenses ([docs/FREE-3D-MODELS.md](docs/FREE-3D-MODELS.md)) | Owner decision 2026-10-13 |

The full comparison of languages and engines — C++, Rust, Zig, Odin, Godot, Bevy — is in [docs/LANGUAGES.md](docs/LANGUAGES.md).

## 5. Gameplay principles

- **Phases change rules, not just lighting.** Morning is economy and crafting; day/evening is social and physical; night is action and stealth. Systems belong to a phase and must be legible in it.
- **Fail states are stories.** Spilled drink, a shot fired, a raid — the game remembers and the factions react. Reload is not the only answer.
- **Information is the real currency.** Overheard conversations, recipes, debts: knowing more beats shooting more.
- **NPCs are people before they are quests.** Every regular has a routine, a taste and a line you can cross.
- **Readability over realism.** Meters and tells exist; a noir mood never hides whether a customer is about to break a bottle over your head.

## 6. Narrative principles

- The twist (the uncle, the recipes, the city) is the spine: everything else hangs on it.
- Factions are mirror images: the detective, the spy, the mafia boss — each offers help that costs more than it gives.
- Choices are recorded, not labelled: no good/evil meter, only consequences that surface later.
- The player character talks through drinks first and words second.

## 7. Interface and feel

- One screen, no HUD soup: patience, mood and threat are diegetic or minimal.
- Controller, mouse and keyboard both first-class on PC.
- Full language toggle EN ↔ RU in-game, mirroring this repository's documentation.
- Accessibility: subtitles with speaker labels, remappable keys, one setting that calms camera shake and flashes (the bar can get loud).

## 8. Performance and quality bar

- 60 fps at 1440p on a mid-range GPU is the target; ray tracing is optional.
- Loading exists between phases, never inside a conversation.
- Every ship build passes: no crash in a full day-cycle, no blocking bug in the phase loop, assets licensed and logged in [docs/FREE-3D-MODELS.md](docs/FREE-3D-MODELS.md).

## 9. Order of work

This file first; then ARCHITECTURE.md and ROADMAP.md detailed enough to hand to an agent; then code, with the owner checking each step. Every documentation change updates its language twin and is recorded in CHANGELOG.md — the routine is in [docs/MAINTENANCE.md](docs/MAINTENANCE.md).
