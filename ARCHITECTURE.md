# Architecture

How LAST CALL is built. [PHILOSOPHY.md](PHILOSOPHY.md) says what it is and why; where they disagree, that file wins. [ROADMAP.md](ROADMAP.md) says in what order this is built.

Target: **Unreal Engine 5 (5.4+), C++ primary, Blueprints for iteration, PC first.**

## 1. Project shape

One UE5 project, one game target, tools outside the runtime:

```
LastCall/                          Unreal project
  Source/LastCall/                 game module (C++)
    Core/                          phase clock, save, services, events
    Bar/                           pouring, inventory, upgrades, crafting
    Social/                        customers, moods, dialogue, factions
    Night/                         stealth, combat, debt collection, raids
    Coop/                          replication, roles, session flow
    UI/                            HUD, menus, widgets (Blueprint-facing API)
  Source/LastCallTools/            editor-only commands (import, validate, cook)
  Content/                         maps, meshes, materials, DataTables
  Data/                            source of truth: JSON/YAML, in git
tools/                             Rust (or Zig) CLIs: pipeline, linters
docs/                              design documentation, bilingual
```

Two rules keep this from rotting:

1. **Design data lives in `Data/` as text** (dialogues, recipes, customers, balance) and compiles into DataTables at build time. Never balance the game only inside `.uasset`.
2. **Anything a designer edits weekly is data; anything with a state machine is C++.** Blueprints are for presentation and wiring, not for rules that must be tested.

## 2. The day loop

The game runs a clock, not a level flow. One session = one in-game day.

```
Morning (Prep)  ->  Shift (Day/Evening)  ->  Night (Action/Stealth)  ->  Summary -> next day
     ^                      ^                        ^
     |                      |                        |
  economy systems      social + liquid          combat/stealth systems
```

`ULCPhaseManager` is the only owner of the current phase. It:

- enables/disables subsystem sets per phase (each subsystem declares the phases it lives in);
- raises `FOnPhaseChanged`, which UI, music and AI listen to;
- persists the phase boundary in the save file, so quitting mid-shift resumes at the same pour.

Phase transitions are **the only loading points** in the game (see PHILOSOPHY §8).

## 3. Core subsystems

| Subsystem | Class prefix | Responsibility |
| :--- | :--- | :--- |
| Phase clock | `ULCPhaseManager` | Phase state machine, scheduling, save boundary |
| Economy | `ULCEconomyComponent` | Money, suppliers, daily stock, loans |
| Liquid / pouring | `ULCLiquidSystem` + `FLCPourSolver` | Bottle tilt, stream, volume, spill, mixing rules |
| Cocktails | `ULCRecipeLibrary` | Recipes (data-driven), combo validation, signature drinks |
| Crafting | `ULCInfusionSystem` | Player-created syrups/infusions as new data rows |
| Customers | `ULCCustomerComponent` | Mood, patience, order, body-language tells |
| Dialogue | `ULCDialogueRunner` | Data-driven trees, conditions, eavesdrop flags |
| Factions | `ULCFactionSystem` | Standing with the three gangs + police heat |
| Skills | `ULCSkillTree` | Flair, Psychology, Steady hands, Alchemy |
| Threat / sixth sense | `ULCThreatSensor` | Aggression scoring, "who is here to kill whom" |
| Night actions | `ULCNightDirector` | Orders to go, debt runs, investigations |
| Raid events | `ULCRaidDirector` | Police search, evidence hiding, distraction checks |
| Bar growth | `ULCVenueUpgrade` | Room layout, VIP zone, floors, casino tier |
| Co-op | `ULCCoopSession` | Roles, replication, delegation under load |
| Save | `ULCSaveSystem` | One slot, day granularity, undo-free (consequences stick) |
| Services | `FLCServices` | Static access point for the above, injected at init |

Every subsystem is an `UObject` component or a plain service — **no gameplay class may reach another subsystem's internals**; cross-system talk goes through the event bus (`FOnPhaseChanged`, `FOnCustomerServed`, `FOnHeatRaised`, …). This is what keeps the phase swap cheap.

## 4. Liquid subsystem (the signature tech)

No engine ships bartender physics, so we build a *convinced* approximation, not a full CFD solve:

- **Pour:** the bottle is a tilted rigid body; `FLCPourSolver` integrates a stream ribbon: tilt angle → flow rate (Bernoulli-ish curve with viscosity per liquid), stream follows gravity with slight spread, and lands on the glass' collision volume.
- **Volume in glass:** each glass tracks per-liquid layers (colour, opacity, foam head). Mixing rules check order, speed and continuity of the pour — that is the combo system, evaluated from solver output, never from button presses.
- **Failure:** overpour increases level; overflow spawns decals + a spill volume the customer reacts to. Too slow → patience drain (see social).
- **Rendering:** Niagara ribbons and metaballs for the stream, refraction/absorption materials for glass, condensation and ice via material layers (see [docs/ART-BIBLE.md](docs/ART-BIBLE.md)).
- **Cost:** the solver runs per actively poured drink only, at fixed 30 Hz with interpolation; no global fluid grid.

Interfaces: `IPourable` (glass), `IPourSource` (bottle, tap, siphon), `FMixingRules` (data: speed windows, layer order, combo keys).

## 5. Social subsystem

- `ULCCustomerComponent` owns **patience** and **mood** as floats, decaying/growing per phase tick.
- **Tells:** a small animation + icon state set ("wants to get drunk", "wants to talk", "here to kill") driven by `ULCThreatSensor`; Psychology skill unlocks the full set.
- **Consequences:** interrupting a story raises faction-visible flags; the customer's reaction table is data (`Data/Customers/*.json`).
- Dialogue is **interruptible**: a pour in progress can cut a line off, and the tree records it. This is a first-class event, not a UI annoyance.

## 6. Night, threat and raids

- Night is a separate mode with its own input set (cover, interact, throw), reusing the bar as a level.
- `ULCRaidDirector` runs a search: evidence items have hide-tags; the NPC partner performs the hiding while the player passes a dialogue/distraction check. Failure = heat, not game over.
- Combat is rare and lethal: shooting exists, but the intended win is de-escalation (drink, talk, block the door).

## 7. Data and dialogue

```
Data/
  Recipes/*.json        cocktail recipes, combo keys, effects
  Customers/*.json      regulars, schedules, stories, reactions
  Dialogue/*.json       trees, conditions, interruptions
  Skills/*.json         skill tree edges and costs
  Balance/*.json        patience/mood tuning, prices, tips
  Localization/         en.json, ru.json — written natively in both languages (owner decision 2026-10-13), game text mirrors the docs' bilingualism
```

- Dialogue trees are JSON with condition nodes; `tools/lc-dialogue-lint` (Rust) checks reachability, missing localization keys and dead ends in CI.
- Localization keys are shared between UI and dialogue; **parity lint fails the build when either language misses a key** — neither language is downstream of the other.

## 8. Multiplayer (co-op 2–4)

- Listen-server, UE replication; the host runs the sim, clients own their role.
- Roles (bartender, waiter, bouncer, DJ) map to replicated components; the crowd is server-side, cosmetics replicated.
- Co-op is a **mode over the same day loop**, not a separate game: phase manager is authority-side only.
- Network failure = pause, never silent desync (design decision; see [docs/GAME-DESIGN.md](docs/GAME-DESIGN.md) §Multiplayer).

## 9. Save and consequences

- One save slot per profile, written at phase boundaries; mid-phase autosave exists but consequences are applied immediately (PHILOSOPHY §5: fail states are stories).
- Save stores: day, phase, money, stock, faction standing, heat, learned recipes, skill nodes, story flags, venue tier.
- Never stored: liquid simulation state (recomputed), cosmetic settings (engine user config).

## 10. Tests

| Kind | What | Tooling |
| :--- | :--- | :--- |
| Unit | pour solver curves, combo evaluation, economy math, dialogue conditions | UE Automation tests, run in CI on Linux + Windows |
| Data | schema validation for everything in `Data/`, dialogue lint, localization completeness | Rust CLIs in `tools/`, run before every build |
| Scenario | a scripted full day: prep → serve N customers → close → night | headless UE run, the ROADMAP gate for milestones |
| Manual | feel of the pour, readability of tells, noir lighting | checklist per milestone in ROADMAP |

A milestone is closed only when its **user path runs end to end** — a scenario test or a live run — not when unit tests pass.
