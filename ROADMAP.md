# LAST CALL roadmap

PC first; the campaign ships before co-op; a milestone has no date. **v1.0 ships when Track A and Track C are complete** — the day loop plays end to end with the story spine in it, on content we made ourselves.

Status: `[ ]` not started, `[~]` in progress, `[x]` done. Milestones have no dates.

A milestone is closed only when its **user path runs end to end** — a scenario test or a live run — not when unit tests pass (see [ARCHITECTURE.md](ARCHITECTURE.md) §10).

## Track F: foundations (prototype)

- [ ] **F0. Project skeleton (gate).** UE5 project builds on Linux and Windows; `Source/LastCall` modules compile; `Data/` schema validation and dialogue lint run in CI; documentation pairs linked both ways.
- [ ] **F1. One pour.** The pour solver moves a bottle, a stream and a glass: tilt → flow, volume rises, overflow spills, customer reacts. This is the gate for everything else — no other milestone starts before a stranger can spill a drink on camera.
- [ ] **F2. One day.** `ULCPhaseManager` runs Morning → Shift → Night → Summary with placeholder content and a save at each boundary.
- [ ] **F3. One customer.** Patience, mood, one order, one story, one tell; interrupting has a recorded consequence.

## Track A: the bar (core)

- [x] **A0. Documentation set.** Philosophy, architecture, roadmap, changelog, contributing, conduct, security, versioning — in EN and RU, plus the game design, narrative, art, languages, asset catalog and doc routine (2026-10-13).
- [ ] **A1. Liquid core.** Multi-layer glasses, viscosity per liquid, foam head, spill decals, combo evaluation from solver output.
- [ ] **A2. Recipe library.** Data-driven recipes, signature drink, the wall-of-recipes twist hook as data, effects on mood/tips.
- [ ] **A3. Economy.** Suppliers, daily stock, prices, wages, loans; the morning phase is a real decision, not a shop screen.
- [ ] **A4. Customers as people.** Regulars with schedules and tastes, body-language tells, the sixth-sense threat scoring.
- [ ] **A5. Social consequences.** Faction-visible flags for how you treated stories; reaction tables; no good/evil meter.
- [ ] **A6. Skill tree.** Flair, Psychology, Steady hands, Alchemy — four branches, all feeding the pour or the read.
- [ ] **A7. Crafting.** Infusions and syrups as new data rows; experiments that can fail; the signature cocktail questline.
- [ ] **A8. Destructibility.** Bottles as weapons, flipped tables, siphon as extinguisher — interaction set used by both shift and night.

## Track B: the night (action / stealth)

- [ ] **B1. Night mode.** Own input set, cover, the bar as a level after closing.
- [ ] **B2. Orders to go.** The armored window: rapid assembly under time pressure while the city waits outside.
- [ ] **B3. Debt runs.** City blocks for collection; bar tools as weapons; non-lethal wins preferred.
- [ ] **B4. Raid director.** Police search, evidence tags, NPC partner hiding them, distraction dialogue checks.
- [ ] **B5. Investigations.** Eavesdropped conversations as inventory; using information beats using force.

## Track C: the story

- [ ] **C1. Narrative spine.** The uncle, the encrypted recipes, the city: beat sheet approved, written in EN and RU.
- [ ] **C2. The three regulars.** Detective Harris, Miss Kane, the boss «Shadow» — full dialogue trees, schedules, consequences.
- [ ] **C3. Faction arcs.** Three gangs + police: intersecting storylines, no single "correct" loyalty.
- [ ] **C4. Ending set.** Multiple endings from standing + information + venue tier; each closes the loop of the day.

## Track D: growth and co-op

- [ ] **D1. Venue progression.** Dive → upgraded bar → VIP floor(s) → underground casino; layout data, not hardcoded rooms.
- [ ] **D2. Co-op 2–4.** Roles (bartender, waiter, bouncer, DJ), listen server, reconnect-as-pause, crowd load.
- [ ] **D3. Dynamic events pool.** Raids, celebrity visits, rival openings, blackouts — weighted by faction standing.

## Track P: production and release

- [ ] **P1. Vertical slice.** One polished day + one night + one dialogue arc, 15 minutes a stranger can play.
- [ ] **P2. Store presence.** Bilingual store page, trailer, demo build (the demo is the vertical slice).
- [ ] **P3. Playtest loop.** External playtests with the pour-feel and readability checklists from PHILOSOPHY §7–8.
- [ ] **P4. Release build.** Versioning per [VERSIONING.md](VERSIONING.md), asset licenses verified against [docs/FREE-3D-MODELS.md](docs/FREE-3D-MODELS.md), no telemetry (PHILOSOPHY §3).
- [ ] **P5. Post-launch.** Localization fixes, community-reported balance, mod-friendly data (Data/ ships readable).

## How milestones are worked

- **Team:** one developer with AI agents. Milestones are taken strictly in order, one at a time, and worked autonomously end to end — each one lands as a checked result, not a status update.
- One milestone, one branch, one result that can be checked. Within a track, milestones go in order.
- Track F did not start without F0 building in CI; **F1 (the pour) gates every gameplay milestone**.
- Every milestone passes the data linters and the scenario test for its track.
- The owner reviews each step; documentation is updated in the same change (see [docs/MAINTENANCE.md](docs/MAINTENANCE.md)).
