# LAST CALL — Narrative

Story, characters and factions. Tone rules are in [PHILOSOPHY.md](PHILOSOPHY.md) §6; the beats feed the systems in [GAME-DESIGN.md](GAME-DESIGN.md).

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
- **Bilingual from birth:** dialogues and lore are written natively in **both** English and Russian (owner decision, 2026-10-13) — not drafted in one and translated into the other; each language is authored on its own. Strings live in `Data/Localization/` (`en.json`, `ru.json`), and the parity lint fails the build if either side misses a key ([ARCHITECTURE.md](ARCHITECTURE.md) §7).

## 8. Content checklist for v1.0

- [ ] Act I–IV beat sheet, written and approved (ROADMAP C1)
- [ ] 3 major characters: full trees + schedules (C2)
- [ ] 12 regulars: 3–6 scenes each (C3)
- [ ] 4 factions: standing events + reaction tables (C3)
- [ ] 6 endings, each with a distinct last night (C4)
- [ ] RU pass by a native editor, not machine-only (P3)
