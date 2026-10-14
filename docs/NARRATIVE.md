# LAST CALL — Narrative
**[Русская версия этого файла →](NARRATIVE.ru.md)**

Story, characters and factions. Tone rules are in [PHILOSOPHY.md](../PHILOSOPHY.md) §6; the beats feed the systems in [GAME-DESIGN.md](GAME-DESIGN.md).

## 1. Premise

Alex inherits a bar in the center of the metropolis from an uncle they barely knew. The bar is **neutral territory for three criminal groups** — and a hidden communication node. After closing, the basement bunker holds illegal equipment. And the uncle? He was the **keeper of information that can destroy the city**, encrypted in the cocktail recipes chalked on the wall. Everyone wants it: the gangs, the detective, the woman who always orders a martini.

**Formula:** *Papers, Please* meets *Grand Theft Auto* in the décor of a chaotic wedding comedy.

## 2. The spine (acts)

| Act | Days | The player learns | The twist of the act |
| :--- | :--- | :--- | :--- |
| **I. Inheritance** | 1–7 | Pouring, customers, the three factions' habits | The recipes on the wall are not decoration — someone breaks in to read them |
| **II. Neutral ground** | 8–15 | Debt, favors, the detective's case, the bunker | The uncle was selling information to all three factions at once |
| **III. Pressure** | 16–23 | Raids, night work, the real content of the archive | The archive is keyed to the drinks: only a bartender who can make them all can open it |
| **IV. Last call** | 24–30 | Everyone converges on the bar | Who you served, who you ignored, what you poured decides the ending |

Endings (6) are computed from **faction standing × information collected × venue tier**. No good/evil axis (PHILOSOPHY §6).

### 2.1 Beat sheet (draft, ROADMAP C1 — awaiting owner approval)

Beats reference the systems that already carry their weight in data: story flags (`heard_harris_case`, `poured_harris_before_talk`, `heard_harris_meeting`, `ignored_harris_meeting`), the secret fragments on the wall (`secret.fragment 1..13`), intel rows (`docks_meet`, `shipment`, `bought_cop`), night activities (to-go orders, debt runs, investigations) and the raid rules.

**Act I — Inheritance (days 1–7)**

| # | Beat | What it sets in motion |
| :--- | :--- | :--- |
| 1 | **Keys and chalk.** Morning of day 1: the bar, the uncle's notebook, the wall of recipes. Signature Noir is the first drink Alex can actually reproduce. | `act1_hook`, the recipe book becomes the quest log |
| 2 | **First shift, first regular.** Detective Harris takes the third stool; the pour before the talk is noticed. | `poured_harris_before_talk`, mood, the Bureau's first standing move |
| 3 | **The case on the counter.** Harris asks for ears. Listening, pouring first or turning him down all register. | `heard_harris_case` / `ignored_harris_meeting`, the investigations activity opens |
| 4 | **Three habits.** The Syndicate's drop, the Ring's delivery, the Station's blind eye — each established by one unremarkable evening. | faction baselines, reaction-table flags |
| 5 | **Thursday.** Shadow arrives alone, orders exactly from the uncle's ledger. Serve it right and the shield holds. | Shadow trust, act II's fork |
| 6 | **The martini with one olive.** Miss Kane trades a true fact for a dry martini and silence. | the espionage line, her arc's first scene |
| 7 | **Someone reads the wall.** A break-in at closing: nothing stolen but a photograph of the chalk. | act I twist, the gangs now know Alex has the wall |
| 8 | **The notebook's last page.** "The archive is not here. It is in what I made people drink." | act II hook, the 13 fragments questline |

**Act II — Neutral ground (days 8–15)**

| # | Beat | What it sets in motion |
| :--- | :--- | :--- |
| 9 | **Night work begins.** To-go orders at the shutter; the first debt run — talk, intimidate or take. | B2/B3 loops, non-lethal preference taught by rewards |
| 10 | **The bunker under the bar.** The uncle's basement is a functioning comms node. | the archive becomes physical, Station interest rises |
| 11 | **Sold to everyone.** The uncle was selling information to all three factions at once — and charging each for the silence of the others. | act II twist, every faction has a reason to fear the wall |
| 12 | **Kane's price.** She knew the uncle; some recipes were written for her. She offers the third fragment for one honest answer. | her arc's fork: asset or loose end |
| 13 | **The first raid.** Evidence tags, the partner hiding them, a distraction check at the door. | heat, fines, the Bureau or the Station moving |
| 14 | **Shadow's test.** A wrong drink on a Thursday, or a loyalty no one asked for. | trust kept or spent — act III's engine either way |
| 15 | **The archive speaks.** Playing the fragments back on the bunker equipment: the archive is keyed to the drinks — only a bartender who can make them all opens it. | act II twist, the 13-recipe main quest made explicit |

**Act III — Pressure (days 16–23)**

| # | Beat | What it sets in motion |
| :--- | :--- | :--- |
| 16 | **Raid season.** The morning pool now rolls raids often; hiding is a routine, not an event. | standing-driven difficulty, the partner becomes load-bearing |
| 17 | **The bought cop's name.** Intel `bought_cop` turns a Station problem into a lever — or a threat. | Station arc fork, using information beats using force |
| 18 | **The docks and the shipment.** `docks_meet` and `shipment` let the player choose who hears what. | Ring and Syndicate arcs intersect; selling intel is possible and remembered |
| 19 | **Shadow unmasked — or not.** His secret: he is not the top. The real boss is in the room every night as someone else. Who it is depends on who the player served. | act III twist, the final boss is chosen by the player's habits |
| 20 | **The night it costs.** A death (or a hospital stay) is not a reload: heat, standing, days gone. | B1 consequences felt in the story, not the UI |
| 21 | **Regulars cash in.** Each of the twelve arcs pays its favor or its betrayal, exactly when it hurts or helps most. | the sandbox's connective tissue deciding act IV's crowd |
| 22 | **The thirteenth fragment.** The last piece is not on the wall and not in the archive — it is what the uncle used to make Shadow's drink. | act III close, the final recipe becomes the key |

**Act IV — Last call (days 24–30)**

| # | Beat | What it sets in motion |
| :--- | :--- | :--- |
| 23 | **The converging week.** Everyone is at the door every night: orders to go spike, debt runs turn dangerous, the shutter stays half down. | all three night loops at maximum |
| 24 | **The last bartender test.** The full wall, one night: every recipe in sequence, no misses. | the archive opens — or the key is lost trying |
| 25 | **Last call.** The final shift is a service, not a battle: who gets served, who gets refused, what gets poured decides who walks out of the bar. | the ending reads the consequence log, not a combat result |
| 26 | **Six endings.** Standing × collected information × venue tier; each ending has its own last night and its own morning after. | C4, replayability from decisions, not difficulty |

Drafted 2026-10-14; approval requested from the owner (ROADMAP C1).

## 3. Major characters

### Alex — the player
Bartender, late twenties/early thirties. Inherited, not chosen. Speaks through drinks. Voiced sparsely: the player's choices are mostly *what gets poured and to whom*.

### Detective Harris — the law that drinks whiskey
- **Orders:** whiskey, always the same glass, always the same seat.
- **Line:** trying to close the mafia case; he wants the bar's ears.
- **Arc:** helping him cleans the street but paints a target on the bar; refusing him costs information and safety.
- **Secret:** his case evidence is stored where the gangs can reach it — he is more compromised than he admits.

### Miss Kane — the woman who orders a martini with an olive
- **Orders:** dry martini, one olive, never a second drink.
- **Line:** knows everything about everyone; she trades in facts.
- **Arc:** her information is real and always costs something; the espionage line culminates in her choosing the player as an asset — or a loose end.
- **Secret:** she knew the uncle; some of the recipes were written for her.

### «Shadow» — the boss who comes on Thursdays
- **Orders:** the exact drink from the uncle's ledger. Getting it wrong loses "protection" — or worse.
- **Line:** arrives alone, speaks little, everyone else goes quiet.
- **Arc:** his trust is the bar's shield; his betrayal is act III's engine.
- **Secret:** he is not the top of his own organization — the real boss is in the room every night as someone else.

## 4. The regulars (12 arcs)

Besides the three majors, twelve regulars carry smaller arcs that intersect the factions: the office worker who launders money through tips, the bartender's ex-partner, the union rep, the street preacher, the hacker who uses the bar's Wi-Fi, the bartender's uncle's former partner, the paramedic, the bartender's mother's friend (the bar's landlady), the DJ, the pickpocket kid, the ex-boxer doorman-waiting-to-be-hired, the journalist.

Each arc: 3–6 scenes, one favor, one betrayal option, one ending state. They are the sandbox's connective tissue.

## 5. Factions

| Faction | Wants | Gives | Turns if... |
| :--- | :--- | :--- | :--- |
| **The Syndicate** (gang) | A quiet place, a message drop, no police | Protection, night orders, cash | The player serves their enemies at the same counter |
| **The Ring** (rivals) | Territory; the bar is in the way | Information, debt leverage | The player refuses their deliveries three times |
| **The Bureau** (detective + honest cops) | Evidence, ears, an inside man | Warning of raids, legal cover | The player tips off the gangs — permanently |
| **The Station** (bought cops) | A cut, and the bunker's contents | Turned a blind eye, until the price rises | The player stops paying or gets greedy |

Standing is a number per faction; heat (police attention) is separate. Both are visible only as consequences (no reputation HUD, PHILOSOPHY §7).

## 6. Storytelling rules

1. **The recipes are the plot device.** Every recipe learned is a piece of the key; the recipe book doubles as the quest log.
2. **No exposition dumps in the shift.** Lore arrives as overheard fragments, customer stories, and the uncle's notebook in the morning phase.
3. **Interrupting is a choice.** The dialogue system records every ignored story (ARCHITECTURE §5) and pays it back in act III–IV.
4. **Consequences are delayed.** Nothing shows a "you chose wrong" screen; the reaction arrives days later, at the door, at night.
5. **The bar is the character.** Its state — clean/damaged, loud/quiet, dive/VIP — is described in every scene that happens in it.

## 7. Writing style

- Hard-boiled but warm: short sentences at the bar, long ones in the confessions.
- No purple prose about rain; the rain is in the renderer (see [ART-BIBLE.md](ART-BIBLE.md)).
- Curses kept mild (rating target: 16+); violence described by consequence, not by gore.
- Every line must survive being said by a person holding a bottle.
- **Bilingual from birth:** dialogues and lore are written natively in **both** English and Russian (owner decision, 2026-10-13) — not drafted in one and translated into the other; each language is authored on its own. Strings live in `Data/Localization/` (`en.json`, `ru.json`), and the parity lint fails the build if either side misses a key ([ARCHITECTURE.md](../ARCHITECTURE.md) §7).

## 8. Content checklist for v1.0

- [~] Act I–IV beat sheet: drafted in EN and RU (§2.1), awaiting owner approval (ROADMAP C1)
- [ ] 3 major characters: full trees + schedules (C2)
- [ ] 12 regulars: 3–6 scenes each (C3)
- [ ] 4 factions: standing events + reaction tables (C3)
- [ ] 6 endings, each with a distinct last night (C4)
- [ ] RU pass by a native editor, not machine-only (P3)
