# LAST CALL — Game Design Document

The authoritative design document. Where it conflicts with [PHILOSOPHY.md](PHILOSOPHY.md), philosophy wins; implementation details live in [ARCHITECTURE.md](ARCHITECTURE.md).

**Genre:** story-driven management sim with action and RPG elements · **Platform:** PC · **Camera:** first/third-person hybrid behind the bar, over-the-shoulder at night · **Players:** 1, co-op 2–4 · **Session:** one in-game day (30–60 min), phases skippable at day end.

## 1. Core loop

```
 ┌─ MORNING: buy, upgrade, craft ──────────────┐
 │                                              │
 ▼                                              │
 SHIFT: pour • listen • read the room           │
 │                                              │
 ▼                                              │
 NIGHT: deliver • collect • investigate         │
 │                                              │
 ▼                                              │
 SUMMARY: tips, XP, faction standing, heat ─────┘
```

The player's real resource is **attention**; money and XP are its byproducts. Every phase spends attention differently: economy in the morning, people in the evening, danger at night.

## 2. Phase 1 — Prep (morning)

| System | Design |
| :--- | :--- |
| Purchasing | Suppliers with stock limits and prices that drift; cheap booze raises night risk (worse customers), premium raises margin. Ice runs out if forgotten. |
| Upgrades | Tap repair, mixer, VIP zone, lighting, jukebox → each unlocks a mechanic or a customer segment, each has a visible state in the bar. |
| Crafting | Infusions/syrups: pick ingredients, time and strength. Results are new recipe rows. Failures are usable (wrong drink, funny effect) or dumpable. |
| Reputation preview | The day's expected crowd is shown (office crowd, gang night, detective's day) — the prep is a bet on who comes. |

Timer: relaxed by default; a "rush" toggle for players who want pressure. Morning never blocks the day.

## 3. Phase 2 — Shift (day/evening)

### 3.1 Pouring (the signature mechanic)

- **Input:** analog tilt — controller stick or mouse drag. Tilt sets flow rate; the stream is a simulated ribbon; the glass fills in layers.
- **Skills it tests:** steadiness (keep the stream in the glass), timing (stop at the line), knowledge (right drink, right order), speed (patience drains).
- **Failures:** overpour (waste, cheap drink), spill (decal, customer reaction), wrong drink (re-pour, patience hit), too slow (boredom → mood loss).
- **Cocktail-combo:** correct sequence + speed windows + continuity = *critical success*: mood buff for the customer, bonus tips and XP. Combos are discovered through experimentation and recorded in the recipe book.
- **Flair:** unlocked skill — juggling/throws that skip animations and raise tips; risk of dropping when pressured.

### 3.2 Customers

Each customer carries: **order**, **patience** (bar), **mood** (bar), **story state**, **tell set**.

- **Patience** drains on waits, spills, wrong drinks; refills on correct service and on being listened to.
- **Mood** scales from irritated to delighted; affects tips, gossip quality, and whether they start a fight or calm one.
- **Tells** (body language): `thirsty` (wants volume), `lonely` (wants talk), `hostile` (here for someone), `nervous` (has info), `fake` (performing). The Psychology skill reveals which one; without it, the player reads animation only.
- **Listening:** a customer mid-story can be interrupted (pour) or attended (stop and look). Interrupting records `ignored_<id>`; attending records `heard_<id>`. Both open and close later scenes.

### 3.3 Sixth sense / threat

A scoring function (`ULCThreatSensor`) ranks hostility in the room. At high score:

- music ducks, ambient chatter fades (audio tell);
- a highlighted seat/patron shows in the periphery when the Steady-hands/Psychology skills are taken;
- the player can pre-empt: pour a calming drink, place a bouncer, block an aisle — or start it themselves when unavoidable.

### 3.4 Cocktail-combo system (detail)

| Layer | Rule |
| :--- | :--- |
| Sequence | Base → modifier → accent (data-defined per recipe) |
| Speed | Each pour has a flow window; outside it, quality drops |
| Continuity | Pauses > N seconds break the combo |
| Result | quality ∈ {ruined, fine, good, critical} → mood/tip/XP multipliers |

Quality is computed by `ULCRecipeLibrary` from solver output — never from button presses (PHILOSOPHY §3.3).

## 4. Phase 3 — Night (action/stealth)

Available depending on story flags:

1. **Orders to go.** An armored window opens for a fixed window of time; a queue of gang orders must be assembled fast while outside pressure builds (timed, stressful, purely mixing).
2. **Debt runs.** Travel through city blocks (small hand-built levels): find the debtor, choose talk / intimidate / take something. Bar tools are the weapons: bottle, shaker, siphon, tray.
3. **Investigations.** Eavesdrop tokens collected during shifts are spent to unlock intel: who meets whom, which shipment, which cop is bought. Intel changes faction arcs.

Night combat rules: rare, lethal, avoidable. Shooting exists; the intended wins are de-escalation, escape, or a very good thrown bottle. Death = consequences (heat, reputation, hospital days), not a reload prompt.

## 5. Systems

### 5.1 Skill tree — four branches

| Branch | Effect |
| :--- | :--- |
| **Flair** | Bottle juggling, throw-pours: faster service, higher tips, spectacle raises mood; drops under pressure. |
| **Psychology** | Reveals tells, hidden desires, and lie detection in dialogue; shows intent icons. |
| **Steady hands** | Reduces aim/pour shake during gunfire or panic; steadier stream, longer combo windows. |
| **Alchemy** | Unlocks "special ingredients": sedatives, laxatives, poisons — each with faction consequences and discovery risk. |

XP comes from served customers, critical combos, resolved conflicts and night objectives. Skills cost XP + a prerequisite.

### 5.2 Dynamic events

Weighted pool, evaluated each morning by faction standing, heat and day count:

- **Police raid:** evidence items get hide-tags; NPC partner hides them while the player passes a distraction dialogue check. Failure = heat + fines, not game over.
- **Rival bar opening / gang meetup / celebrity visit / blackout:** each shifts the crowd, the orders and the music.

### 5.3 Destructibility and environment

- Breakable bottles (improvised weapon, hazard), throwable trays, tables that flip into cover.
- Soda siphon as extinguisher/fog screen; spilled alcohol + lighter = fire (rare, dramatic).
- The bar remembers damage: repairs cost money and look different until fixed.

### 5.4 Economy and progression

Money sinks: stock, wages, repairs, upgrades, bribes. Growth ladder:

`dive → renovated bar → VIP floor → second floor → underground casino`

Each tier: new room (layout data), new crowd, new risk, new music set. The tier also feeds the ending (see [NARRATIVE.md](NARRATIVE.md)).

### 5.5 Multiplayer (co-op 2–4)

| Role | Job |
| :--- | :--- |
| Bartender | Pouring, combos, dialogue (the whole single-player loop) |
| Waiter | Carrying, seating, clearing, relaying orders |
| Bouncer | Door, threat, crowd control, night security |
| DJ | Music set, mood of the room, event cues |

- Listen server; one phase authority (host).
- Under load the roles split naturally: the bartender can't seat people, the bouncer can't pour — coordination is the design.
- Failures stay social: a bad night costs money, never kicks players out.
- Reconnect = the room holds; no restart of the day.

## 6. Onboarding and difficulty

- Day 1 tutorial is diegetic: the uncle's notebook (also the recipe book) teaches pouring, listening and the night window.
- Difficulty sliders: patience drain, combat damage, financial pressure, aiming assistance — no "story mode" that removes the pour (the pour is the game).
- Accessibility: subtitle speaker labels, remappable keys, camera-shake/flash damping toggle, high-contrast tells.

## 7. Content scale (v1.0 targets)

| Content | Target |
| :--- | :--- |
| In-game days | 30 story days + repeatable sandbox days |
| Regulars with arcs | 12 (3 majors: Harris, Kane, Shadow) |
| Recipes | 24 base + player-crafted variants |
| City night blocks | 6 hand-built |
| Endings | 6, from standing × information × venue tier |
| Co-op maps | the same bar, 4 roles |

## 8. Anti-goals

- No live-service seasons, no battle pass (PHILOSOPHY §3.1).
- No procedural story: hand-written beats, weighted events only.
- No gunplay as the main verb: the main verb is pouring.
