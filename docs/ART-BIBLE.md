# LAST CALL — Art bible
**[Русская версия этого файла →](ART-BIBLE.ru.md)**

The look and sound of the game. Tone and pillars: [PHILOSOPHY.md](../PHILOSOPHY.md); systems this art serves: [GAME-DESIGN.md](GAME-DESIGN.md).

**One line:** *Realism with noir elements — neon light, rain outside the window, cigarette smoke.* The bar is warm and dirty; the city outside is cold and wet.

## 1. Reference frame

| Layer | Reference | What we take |
| :--- | :--- | :--- |
| Interiors | Old-school bars, *Blade Runner 2049* interiors, *Disco Elysium* mood (not style) | Amber practical light, long shadows, smoke catching beams |
| Exterior | Rain-slick streets, neon signage reflected in puddles | Wet asphalt as a mirror, cold cyan/magenta vs. warm interior |
| Characters | Grounded, ordinary faces; nobody is a supermodel | Silhouette readability first, detail second |
| UI | Bar hardware: chalkboards, receipts, coasters, ledger pages | Diegetic, monochrome + one accent |

Explicitly not: cyberpunk chrome, cartoon stylization, horror gore (PHILOSOPHY §3.5).

## 2. Rendering setup (UE 5.8)

Per [docs/LANGUAGES.md](LANGUAGES.md) §8:

- Lumen GI, **Lumen Lite** on perf-heavy scenes; hardware Lumen where it pays.
- **MegaLights** for the many practical lights of the bar (lamps, neon, TV glow).
- Nanite everywhere for geometry; Virtual Shadow Maps.
- **Local volumetric fog** — the smoke/cigarette layer, always on indoors.
- Substrate materials for wet surfaces; TSR/DLSS/FSR upscaling; PSO precaching on.
- Target: 60 fps @ 1440p mid-range GPU (PHILOSOPHY §8); ray tracing optional.

## 3. Lighting: the three moods

Every space has exactly one dominant mood at a time; the phase manager switches them.

| Mood | Palette | Used in |
| :--- | :--- | :--- |
| **Day shift** | Amber #E8A24A, dusty teal shadows, daylight shafts through blinds | Morning prep, office crowd |
| **Night shift** | Neon magenta #FF3D7F + cyan #33E0FF, sodium-orange street spill, hard pools of light | Evening rush, gang hours |
| **Danger** | Single hard source, deep blacks, one red practical (exit sign, brake light through window) | Threat spikes, raids, night missions |

Rules:

1. **Every light source is motivated** — a lamp, a sign, a window. No floating fill.
2. **The bar counter is always the brightest object in frame** — the eye must land there first during a shift.
3. **Neon sign is the bar's face.** It flickers when faction standing drops; it is broken in the dive state and clean in VIP.
4. **Police light** (rotating blue/red through the window) is a reserved storytelling light: only raids and night events use it, so it always means something.

## 4. Materials

| Surface | Recipe |
| :--- | :--- |
| Wet asphalt | Low roughness + puddle masks + reflection of neon; animated rain ripples |
| Bar counter wood | Decades of polish: layered roughness, ring stains, edge wear — never uniform |
| Glass | Refraction + absorption tint per liquid, condensation layer near the rim, fingerprints |
| Liquid | Per-layer colour/opacity/foam; sugar density changes how it clings |
| Smoke | Volumetric, lit by practicals; cigarette smoke is thinner than fog |
| Brass/chrome | Tarnish in crevices, high polish on touch points (tap handles, siphon) |

## 5. VFX inventory (minimum for v1.0)

Pour stream (metaball ribbon) · splash particles · overflow spill + decal · foam head pop · glass clink sparks (light) · cigarette/smoke volumes · rain sheet + puddle splashes · neon flicker · bottle break (shards + wet decal) · police strobe · dust in light shafts · breath in cold night air.

## 6. Animation principles

- **Weight first:** bottles have liquid inside — the pour is a hip-and-shoulder motion, never a wrist flip (except Flair, which is deliberately theatrical).
- **Service choreography:** pour → present → wipe the counter; this idle loop is the game's heartbeat.
- **Tells are readable at 3 m:** posture shifts, glass grip, head orientation — the threat animation set must read without UI (PHILOSOPHY §7).
- **Combat is ugly and short:** two beats maximum, then separation.
- Faces: MetaHuman or equivalent for the 3 majors; performance-capture-quality is not required, but eye contact in dialogue is.

## 7. UI / HUD

- Diegetic first: chalkboard menu, paper order pads, the uncle's notebook (recipe/quest log), a wall ledger for money.
- Meters (patience, mood) as **thin underline strokes under the customer's name plate**, not floating bars.
- Threat is never a number; it is the room going quiet.
- Colour: warm paper #E7DCC6, ink #1A1614, one accent per faction.
- Type: condensed grotesque for signage, monospace for ledgers; both languages (EN/RU) tested at UI size — Cyrillic must not fall back to a substitute font (a documented failure: always ship the RU font).

## 8. Audio direction

| Layer | Design |
| :--- | :--- |
| Beds | Muted jazz (upright bass, brush drums) → crossfades to tension drone when a "bad guy" enters; the *absence* of music is a cue |
| Bar Foley | Chair creaks, glass clink, pour and fizz, cash drawer, ice — each is a reward or a warning |
| City | Rain on windows, distant siren, traffic hum — always present, louder at night |
| Mix rule | During a customer's story: duck everything except voice + pour; during danger: duck music, keep foley sharp |
| Music rights | Compose original jazz/noir cues (licensed libraries are a fallback, check license terms in [FREE-3D-MODELS.md](FREE-3D-MODELS.md) spirit: known license, recorded) |

## 9. What "done" means for art (v1.0)

- [ ] Three lighting moods implemented and switchable by the phase manager
- [ ] All VFX in §5 present and readable at gameplay distance
- [ ] Telling stories without UI (the §6.3 tell test: a stranger names the threat)
- [ ] EN/RU UI strings fit every plate at 1080p and 4K with the shipped fonts
- [ ] 60 fps @ 1440p target held in the vertical slice scene
