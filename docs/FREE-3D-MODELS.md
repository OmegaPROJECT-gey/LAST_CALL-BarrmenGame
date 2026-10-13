# LAST CALL — Free 3D Models Catalog

Curated **free** assets for the noir bartending sim: bar interior, characters, night streets, vehicles, textures and UE5-native sources. Tone reference: [PHILOSOPHY.md](../PHILOSOPHY.md), content slots: [GAME-DESIGN.md](GAME-DESIGN.md).

**Verified:** 2026-10-13. Every URL below was fetched during the pass (HTTP 200, or 202 for Sketchfab's JS shell). Re-check licences before shipping — free ≠ forever.

## 1. How to read this catalogue

- **License** column: `CC0` = no attribution, commercial OK. `CC-BY` = attribution required. `CC-BY-SA` = attribution **+ share-alike** (avoid in a closed game). `Free` = free under the platform's own EULA (Fab, Mixamo, Marketplace).
- **Format** column: what the source offers. For Sketchfab, confirm the *Downloadable* toggle on the model page — the exact file set is set by the uploader.
- **Rigged?** is called out in Notes: `rigged` = skinned skeleton included, `anim` = animation clips included.
- Anything with `⚠` in Notes needs a human decision before use.

### Licence summary (commercial-game safety)

| Licence | Use in LAST CALL | Attribution | Notes |
| :--- | :--- | :--- | :--- |
| CC0 (Poly Haven, ambientCG, Quaternius, Kenney, Poly Pizza) | ✅ Free | No | Safest tier — nothing to credit |
| CC-BY 4.0 (Sketchfab) | ✅ Free | Yes, in credits | Keep a running `CREDITS.txt` |
| CC-BY-SA (Sketchfab, a few OGA) | ⚠ | Yes + derivative must stay SA | Prefer CC-BY/CC0 alternatives where possible |
| CC-BY-NC / NC-ND / GPL-only | ❌ Excluded | — | Not commercial-safe; filtered out of this list |
| Fab / Marketplace / Mixamo EULA | ✅ Free | No (EULA governs) | Free *within* UE projects and shipped games; check "use outside UE" for Mixamo FBX |
| Ripped game assets | ❌ Excluded | — | Obvious pirated re-uploads were dropped during the scan |

---

## 2. Bar interior & props

Counter, bottles, glassware, stools, shelves, neon. The core dressing of the venue.

| Category | Asset name | Source | URL | License | Format | Notes |
| :--- | :--- | :--- | :--- | :--- | :--- | :--- |
| Bar interior | Pub interior | Sketchfab | https://sketchfab.com/3d-models/none-a6534141c33343b382dd23d519c3de73 | CC-BY 4.0 | glTF / FBX / OBJ\* | Full pub room: counter, back bar, stools. Not rigged. Best blockout base |
| Bar counter | bar counter | Sketchfab | https://sketchfab.com/3d-models/none-78c700db0d9f4d7c800fb789dfa2e057 | CC-BY 4.0 | glTF / FBX / OBJ\* | PBR counter, standalone — easy to drop into a modular room |
| Bar counter | BarCounter - 8192px² | Sketchfab | https://sketchfab.com/3d-models/none-f3646304fee941c19a6086d033ecd9d2 | CC-BY 4.0 | glTF / FBX / OBJ\* | 8K textures — hero close-up quality for the "pour" camera |
| Bar stool | Worn Bar Stool | Sketchfab | https://sketchfab.com/3d-models/none-587a6f10e74f4f8298f6ad15b42bb7cf | CC-BY 4.0 | glTF / FBX / OBJ\* | Worn/damaged look — fits noir grime |
| Bar stool | Bar Stool and Table | Sketchfab | https://sketchfab.com/3d-models/none-880b2ead4b19445896522bd9b6e46517 | CC-BY 4.0 | glTF / FBX / OBJ\* | Set of two — tables for the patron seating rows |
| Shelving | Wooden Back Bar Cabinet - 4096px² | Sketchfab | https://sketchfab.com/3d-models/none-a1961b9e5a7043fb9a70b855e947e27f | CC-BY 4.0 | glTF / FBX / OBJ\* | 4K back-bar shelving; the wall behind Alex |
| Neon | Neon Bar Sign | Sketchfab | https://sketchfab.com/3d-models/none-52d50a3b04ae414cb11b16df0d779054 | CC-BY 4.0 | glTF / FBX / OBJ\* | Emissive — hook to a UE emissive material + flicker |
| Glassware | Cocktail Glass | Sketchfab | https://sketchfab.com/3d-models/none-4f7eeaf5583940d4a5368a431b5a2a8a | CC-BY 4.0 | glTF / FBX / OBJ\* | Martini glass — Miss Kane's drink |
| Glassware | Glass Collection | Sketchfab | https://sketchfab.com/3d-models/none-0ff8fc5007dd4e9da675109fe7c443d6 | CC-BY 4.0 | glTF / FBX / OBJ\* | Assorted glasses in one file |
| Glassware | The ultimate glass pack | Sketchfab | https://sketchfab.com/3d-models/none-1ea37815fb0145e49487386d6924b341 | CC-BY 4.0 | glTF / FBX / OBJ\* | Largest glass set found — covers every order type |
| Bottles | Whiskey and Wine Low-Poly Bottles | Sketchfab | https://sketchfab.com/3d-models/none-99999f032f624e8cb05651f041dbf8eb | CC-BY 4.0 | glTF / FBX / OBJ\* | Low-poly — cheap for a wall of 40+ bottles |
| Bottles | Bar with bottles | Sketchfab | https://sketchfab.com/3d-models/none-acdb303e9849401f95a973469e0310c3 | CC-BY 4.0 | glTF / FBX / OBJ\* | Pre-dressed shelf — instant back-bar dressing |
| Bottles | Wine Bottles | Poly Haven | https://polyhaven.com/a/wine_bottles_01 | CC0 | FBX / OBJ / Blend / glTF | No attribution. Photoreal — better for close-ups |
| Bar tools | Bar Shaker | Sketchfab | https://sketchfab.com/3d-models/none-ec515bceb65a4d7ea59da147a7ea0da7 | ⚠ CC-BY-SA 4.0 | glTF / FBX / OBJ\* | **Share-alike** — prefer CC0 alternatives for the shipped game |
| Bar tools | Molotov cocktail with lighter | Sketchfab | https://sketchfab.com/3d-models/none-11c68745873848dc8d6b15051d9e678e | CC-BY 4.0 | glTF / FBX / OBJ\* | Faction escalation prop |
| Props | Cash Register | Poly Haven | https://polyhaven.com/a/CashRegister_01 | CC0 | FBX / OBJ / Blend / glTF | Till on the counter |
| Furniture | bar_chair_round_01 | Poly Haven | https://polyhaven.com/a/bar_chair_round_01 | CC0 | FBX / OBJ / Blend / glTF | Round bar chair |
| Furniture | metal_stool_01 | Poly Haven | https://polyhaven.com/a/metal_stool_01 | CC0 | FBX / OBJ / Blend / glTF | Also `metal_stool_02`, `metal_stool_03` |
| Furniture | wooden_stool_01 | Poly Haven | https://polyhaven.com/a/wooden_stool_01 | CC0 | FBX / OBJ / Blend / glTF | Also `wooden_stool_02` |
| Furniture | Ultimate Furniture Pack | Quaternius | https://quaternius.com/packs/ultimatefurniture.html | CC0 | FBX / OBJ / Blend / glTF | Broad indoor dressing in one download |
| Furniture | Ultimate House Interior Pack | Quaternius | https://quaternius.com/packs/ultimatehomeinterior.html | CC0 | FBX / OBJ / Blend / glTF | Walls, doors, props for a modular room |
| Bar set (stylised) | Stylized Low Poly Bar Counter Set with Stools and Shelves | Fab | https://www.fab.com/listings/ec950ae1-4bf5-4957-9ce0-b220932a34c2 | Free (Fab EULA) | FBX / OBJ / Blend | Counter + shelves + bottles + glasses + stools, one kit. Low-poly/stylised, not photoreal |
| Furniture (OGA) | 3D Furniture | OpenGameArt | https://opengameart.org/content/3d-furniture | Check page | Blend / OBJ | Has a `Bar.png` preview — bar furniture set |
| Furniture (OGA) | Tiki Bar Stall | OpenGameArt | https://opengameart.org/content/tiki-bar-stall | Check page | Blend / OBJ | Bar-shaped stall; useful silhouette reference |
| Storage (OGA) | Big wine barrel | OpenGameArt | https://opengameart.org/content/big-wine-barrel | Check page | Blend / OBJ | Cellar/basement dressing for the bunker |

\* Sketchfab download formats vary per uploader — check the model page.

## 3. Characters (bartender, patrons, detective, mafia)

| Category | Asset name | Source | URL | License | Format | Notes |
| :--- | :--- | :--- | :--- | :--- | :--- | :--- |
| Rigged coat model | Man In Coat - Human Rigged Model | Sketchfab | https://sketchfab.com/3d-models/none-cd71038373d5427099a7970f1a1af2df | CC-BY 4.0 | glTF / FBX\* | **rigged**, 1 animation. Coat silhouette = detective/mafia read |
| Rigged coat model | old man in coat - human riged model | Sketchfab | https://sketchfab.com/3d-models/none-e8052f5928ef42f1ae5b5af02eb1a36b | CC-BY 4.0 | glTF / FBX\* | **rigged**, 1 animation. Older patron / the uncle in flashbacks |
| Detective | Character (Type Detective) | Sketchfab | https://sketchfab.com/3d-models/none-b2324023364c458db010fe39e8111f38 | CC-BY 4.0 | glTF / FBX\* | Look-matched to Detective Harris |
| NPC (generic) | rigged character low poly | Sketchfab | https://sketchfab.com/3d-models/none-aa2e761ce168492484f8483717898ec8 | CC-BY 4.0 | glTF / FBX\* | **rigged** — cheap crowd filler |
| Mafia | Boss Mafia Man character | Sketchfab | https://sketchfab.com/3d-models/none-3bdb67fd567546f3bf22ada378fe8356 | CC-BY 4.0 | glTF / FBX\* | Faction boss — «Shadow»-type |
| Mafia | Mafia Boss with Gun – Stylized Noir Gangster | Sketchfab | https://sketchfab.com/3d-models/none-abdb14866d91403cab47feaa5393e2f5 | CC-BY 4.0 | glTF / FBX\* | Explicitly noir-styled; gun included |
| Mafia | English Gangster Character Peaky Blinders Style | Sketchfab | https://sketchfab.com/3d-models/none-e63bb61fa3d54d9b83f544043df30cce | CC-BY 4.0 | glTF / FBX\* | Period gangster — 1920s-30s flavour |
| Mafia | Yakuza | Sketchfab | https://sketchfab.com/3d-models/none-67cc7f612434409db2c3cba6a00294eb | CC-BY 4.0 | glTF / FBX\* | Third-faction look |
| Crowd | 1970 Gangster Soldier | Sketchfab | https://sketchfab.com/3d-models/none-d902a608c8584f32b0fa11b67757ff4a | CC-BY 4.0 | glTF / FBX\* | ⚠ T-pose, **not rigged** — needs rigging |
| Ambient NPC | Night Stalker - male | Sketchfab | https://sketchfab.com/3d-models/none-35626d172abb420498ae5c42a9fea2fb | CC-BY 4.0 | glTF / FBX\* | **rigged**, 2 animations. Street/outsider |
| Ambient NPC | Night Stalker - female | Sketchfab | https://sketchfab.com/3d-models/none-22a9d24975f94e29909d2eff1e44fb40 | CC-BY 4.0 | glTF / FBX\* | **rigged**, 2 animations |
| Base body | Universal Base Characters | Quaternius | https://quaternius.com/packs/universalbasecharacters.html | CC0 | FBX / OBJ / Blend / glTF | **rigged**, retargetable — base for a custom Alex |
| Animation | Universal Animation Library | Quaternius | https://quaternius.com/packs/universalanimationlibrary.html | CC0 | FBX / Blend / glTF | Humanoid rig, retarget to UE mannequin |
| Animation | Universal Animation Library 2 | Quaternius | https://quaternius.com/packs/universalanimationlibrary2.html | CC0 | FBX / Blend / glTF | Second batch — idle/talk cycles |
| Animated NPC | Animated Men Pack | Quaternius | https://quaternius.com/packs/animatedmen.html | CC0 | FBX / OBJ / Blend / glTF | Pre-animated patrons |
| Animated NPC | Animated Women Pack | Quaternius | https://quaternius.com/packs/animatedwomen.html | CC0 | FBX / OBJ / Blend / glTF | Pre-animated patrons |
| Animated NPC | Ultimate Animated Character Pack | Quaternius | https://quaternius.com/packs/ultimatedanimatedcharacter.html | CC0 | FBX / OBJ / Blend / glTF | One-click walk/idle set |
| Crowd | Background Posed Humans Pack | Quaternius | https://quaternius.com/packs/backgroundposedhumans.html | CC0 | FBX / OBJ / Blend / glTF | Static seated/standing bodies — background only |
| Animation | Mixamo (full library) | Adobe Mixamo | https://www.mixamo.com | Free (Mixamo EULA) | FBX | **rigged + anim** — retarget idle/talk/drink clips onto any humanoid. Free for commercial use |
| Crowd (OGA) | Animated Human Low Poly | OpenGameArt | https://opengameart.org/content/animated-human-low-poly | Check page | Blend / FBX | Low-poly animated human for distant NPCs |

## 4. Night city streets & exteriors

| Category | Asset name | Source | URL | License | Format | Notes |
| :--- | :--- | :--- | :--- | :--- | :--- | :--- |
| Street blockout | Cyberpunk Game Kit | Quaternius | https://quaternius.com/packs/cyberpunkgamekit.html | CC0 | FBX / OBJ / Blend / glTF | **71 models** — neon city kit, closest style match to LAST CALL |
| Street blockout | Downtown City MegaKit | Quaternius | https://quaternius.com/packs/downtowncitymegakit.html | CC0 | FBX / OBJ / Blend / glTF | Modular city buildings |
| Street blockout | Modular Streets Pack | Quaternius | https://quaternius.com/packs/modularstreets.html | CC0 | FBX / OBJ / Blend / glTF | Road/sidewalk tiles for the exterior |
| Street dressing | City Kit (Roads) | Kenney | https://kenney.nl/assets/city-kit-roads | CC0 | FBX / OBJ / Blend / glTF / SVG | 90 files — road pieces, clean and readable |
| Street dressing | City Kit (Commercial) | Kenney | https://kenney.nl/assets/city-kit-commercial | CC0 | FBX / OBJ / Blend / glTF | Shops/façades for the block around the bar |
| Street dressing | City Kit (Industrial) | Kenney | https://kenney.nl/assets/city-kit-industrial | CC0 | FBX / OBJ / Blend / glTF | Back-alley industrial dressing |
| Street dressing | City Kit (Suburban) | Kenney | https://kenney.nl/assets/city-kit-suburban | CC0 | FBX / OBJ / Blend / glTF | Outskirts / exit routes |
| Street scene | LOWPOLY CITY STREET PACK | Sketchfab | https://sketchfab.com/3d-models/none-8e1ba8a437c4460eaaa643953eaf79d0 | CC-BY 4.0 | glTF / FBX\* | Whole street set — fast exterior blockout |
| Street scene | Cyberpunk City | Sketchfab | https://sketchfab.com/3d-models/none-3f24e5c5bf924f46b30d9a392afa9624 | CC-BY 4.0 | glTF / FBX\* | Night neon city — lighting mood reference + usable geometry |
| Alley | Alleyway | Sketchfab | https://sketchfab.com/3d-models/none-26cb4d9be2ea476686b5d463af642db5 | CC-BY 4.0 (Free Standard) | glTF / FBX\* | The back alley behind the bar |
| Neon | Neon Open Sign | Sketchfab | https://sketchfab.com/3d-models/none-2efad97b50f34220b2df92fe99c8a2ad | CC-BY 4.0 | glTF / FBX\* | OPEN/CLOSED — ties to bar hours loop |
| Neon | Game Ready Cocktails Neon Sign | Sketchfab | https://sketchfab.com/3d-models/none-1698da3aa0c94ac2bdbc0f38ad178e04 | CC-BY 4.0 | glTF / FBX\* | Cocktail neon — the bar's own sign |
| Neon (Fab) | Cyberpunk animated japanese LED neon sign | Fab | https://www.fab.com/listings/8a251ab5-a714-4101-9016-5a22624220dc | Free (Fab EULA) | See page | Animated LED neon — flicker baked in |
| Neon (Fab) | CC0 - Neon Sign Open | Fab | https://www.fab.com/listings/4e687ce7-9aa0-43df-8868-c17da645bbeb | CC0 | See page | Explicitly CC0 — no attribution |
| Street furniture | Street Lamp | Sketchfab | https://sketchfab.com/3d-models/none-6881aa1e84b047d79860fa9297e05e22 | CC-BY 4.0 | glTF / FBX\* | Wet-night key light sources |
| Street furniture | Victorian Street Lamp | Sketchfab | https://sketchfab.com/3d-models/none-b7cebcfb4b1c4d4a97e22072800704ca | CC-BY 4.0 | glTF / FBX\* | Older city — period noir flavour |
| Street furniture | Traffic Lights [Street Assets Vol. 02] | Sketchfab | https://sketchfab.com/3d-models/none-ab3efdd259a54d4aba5f878eacbe3f20 | CC-BY 4.0 | glTF / FBX\* | Street corner dressing |
| Street furniture | street_lamp_01 | Poly Haven | https://polyhaven.com/a/street_lamp_01 | CC0 | FBX / OBJ / Blend / glTF | Also `street_lamp_02` |
| Street furniture | hanging_industrial_lamp | Poly Haven | https://polyhaven.com/a/hanging_industrial_lamp | CC0 | FBX / OBJ / Blend / glTF | Alley/interior overhead |
| Street furniture | fire_hydrant | Poly Haven | https://polyhaven.com/a/fire_hydrant | CC0 | FBX / OBJ / Blend / glTF | Sidewalk prop |
| Street prop | Debris Papers | Poly Pizza | https://poly.pizza/m/MujITy1NRR | CC0 | FBX / OBJ / glTF | Litter for wet streets |
| Street prop | Dumpster | Poly Pizza | https://poly.pizza/m/PKsbolkZSr | CC0 | FBX / OBJ / glTF | Alley prop |
| Street prop | Traffic Light | Poly Pizza | https://poly.pizza/m/lg9AKWejnF | CC0 | FBX / OBJ / glTF | Low-poly variant |
| Road textures | Road Textures | Kenney | https://kenney.nl/assets/road-textures | CC0 | PNG | Tiling road markings |

## 5. Vehicles, police & weapons

| Category | Asset name | Source | URL | License | Format | Notes |
| :--- | :--- | :--- | :--- | :--- | :--- | :--- |
| Police car | Police Car | Sketchfab | https://sketchfab.com/3d-models/none-5510d0c264a24e1990a578142813894e | CC-BY 4.0 | glTF / FBX\* | Primary squad car outside the bar |
| Police car | POLICE CAR - LOW POLY | Sketchfab | https://sketchfab.com/3d-models/none-0f5b12ea1f474887b9664fd7a171c17a | CC-BY 4.0 | glTF / FBX\* | Cheap background cruiser |
| Police car | Hawkins Police Car | Sketchfab | https://sketchfab.com/3d-models/none-ff2a0433d7544fb6b1e23b719fe72e7f | CC-BY 4.0 | glTF / FBX\* | Period-styled cruiser |
| Police car (Fab) | Police car | Fab | https://www.fab.com/listings/009451b7-15f4-4a95-a29b-cfbc55b23d89 | Free (Fab EULA) | See page | Free-tier Fab cruiser |
| Cars | Police Car | Poly Pizza | https://poly.pizza/m/BwwnUrWGmV | CC0 | FBX / OBJ / glTF | Low-poly, Quaternius |
| Cars | Punk | Poly Pizza | https://poly.pizza/m/BTALZymknF | CC0 | FBX / OBJ / glTF | Faction gang car flavour |
| Cars | Cars Pack | Quaternius | https://quaternius.com/packs/cars.html | CC0 | FBX / OBJ / Blend / glTF | Civilian traffic set |
| Cars | Car Kit | Kenney | https://kenney.nl/assets/car-kit | CC0 | FBX / OBJ / Blend / glTF | Modular low-poly vehicles |
| Public transport | Public Transport Pack | Quaternius | https://quaternius.com/packs/publictransport.html | CC0 | FBX / OBJ / Blend / glTF | Buses/taxis for street depth |
| Weapon | Baseball bat | Poly Haven | https://polyhaven.com/a/baseball_bat | CC0 | FBX / OBJ / Blend / glTF | Faction weapon |
| Weapon | Baseball bat | Sketchfab | https://sketchfab.com/3d-models/none-2669cf86386747449611396c0409db47 | CC-BY 4.0 | glTF / FBX\* | Textured alternative |
| Weapon | service_pistol | Poly Haven | https://polyhaven.com/a/service_pistol | CC0 | FBX / OBJ / Blend / glTF | Detective's sidearm |
| Weapon | Pistol | Poly Pizza | https://poly.pizza/m/J3i9KDQ3kt | CC0 | FBX / OBJ / glTF | Low-poly alternative |
| Barrier | concrete_road_barrier | Poly Haven | https://polyhaven.com/a/concrete_road_barrier | CC0 | FBX / OBJ / Blend / glTF | Also `concrete_road_barrier_02` — police cordon |

## 6. Textures & materials

All **CC0**, downloadable as 1K–4K PNG/PBR sets (albedo, normal, roughness, AO, displacement).

| Category | Asset name | Source | URL | License | Format | Notes |
| :--- | :--- | :--- | :--- | :--- | :--- | :--- |
| Wet asphalt | Asphalt025C | ambientCG | https://ambientcg.com/view?id=Asphalt025C | CC0 | PNG 1K–4K | Tags: *dark / rain / wet* — **the** hero wet-street material |
| Asphalt | Asphalt033 | ambientCG | https://ambientcg.com/view?id=Asphalt033 | CC0 | PNG 1K–4K | Second wet-road variant for blending |
| Road | Road007 | ambientCG | https://ambientcg.com/view?id=Road007 | CC0 | PNG 1K–4K | Road surface with markings |
| Road | Road012A | ambientCG | https://ambientcg.com/view?id=Road012A | CC0 | PNG 1K–4K | Alt road tiling |
| Road | Road013A | ambientCG | https://ambientcg.com/view?id=Road013A | CC0 | PNG 1K–4K | Alt road tiling |
| Brick | Bricks097 | ambientCG | https://ambientcg.com/view?id=Bricks097 | CC0 | PNG 1K–4K | Alley/building walls |
| Brick | Bricks060 | ambientCG | https://ambientcg.com/view?id=Bricks060 | CC0 | PNG 1K–4K | Older, dirtier brick |
| Brick | Bricks089 | ambientCG | https://ambientcg.com/view?id=Bricks089 | CC0 | PNG 1K–4K | Third brick for variation |
| Wood floor | WoodFloor064 | ambientCG | https://ambientcg.com/view?id=WoodFloor064 | CC0 | PNG 1K–4K | Dark parquet — bar floor |
| Wood floor | WoodFloor043 | ambientCG | https://ambientcg.com/view?id=WoodFloor043 | CC0 | PNG 1K–4K | Alt interior wood |
| Planks | Planks037A | ambientCG | https://ambientcg.com/view?id=Planks037A | CC0 | PNG 1K–4K | Counter/shelf wood |
| Metal | Metal038 | ambientCG | https://ambientcg.com/view?id=Metal038 | CC0 | PNG 1K–4K | Shaker, taps, railings |
| Rusted metal | Metal056C | ambientCG | https://ambientcg.com/view?id=Metal056C | CC0 | PNG 1K–4K | Grime for fire escapes/doors |
| Asphalt (alt) | asphalt_02 | Poly Haven | https://polyhaven.com/a/asphalt_02 | CC0 | PNG / EXR 1K–8K | Higher-res road option |
| Materials (broad) | BlenderKit free materials | BlenderKit | https://www.blenderkit.com/asset-gallery/?query=category_subtree:material | Free tier (per-asset licence) | Blend | ⚠ Licence is per asset — filter to *Free* and read each page. Includes glass, asphalt, brick categories |
| Glass material | Glass (free) | BlenderKit | https://www.blenderkit.com/asset-gallery/?query=category_subtree:glass | Free tier | Blend | Bottle/glass shader base |

> **Neon/emissive:** no free emissive-texture pack was needed — build neon directly in UE from an emissive material driven by a curve (flicker) over the sign meshes in §2/§4. Quaternius's Cyberpunk Game Kit ships neon-coloured signage meshes.

## 7. UE5-specific sources

| Category | Asset name | Source | URL | License | Format | Notes |
| :--- | :--- | :--- | :--- | :--- | :--- | :--- |
| VFX | Niagara Examples Pack (50+ systems) | Fab / Epic | https://www.fab.com/listings/0e188eca-4e54-4fb2-a9ed-d8b8a565e600 | Free (Fab EULA) | `.uasset` (UE 5.7) | Smoke, mist cards, sparks, footstep FX — covers bar smoke & street mist |
| Bar kit | Stylized Low Poly Bar Counter Set | Fab | https://www.fab.com/listings/ec950ae1-4bf5-4957-9ce0-b220932a34c2 | Free (Fab EULA) | FBX / OBJ / Blend | Direct-download free bar set (also listed in §2) |
| Neon | CC0 - Neon Sign Open | Fab | https://www.fab.com/listings/4e687ce7-9aa0-43df-8868-c17da645bbeb | CC0 | See page | Free sign mesh |
| Vehicle | Police car (SpatialNeglect) | Fab | https://www.fab.com/listings/009451b7-15f4-4a95-a29b-cfbc55b23d89 | Free (Fab EULA) | See page | Free-tier vehicle |
| Photogrammetry | Quixel Megascans — Free library | Quixel / Fab | https://quixel.com/megascans/free | Free (Fab EULA) | `.uasset` / FBX + textures | Photoreal surfaces, props, decals for the wet-street look |
| Search | Fab — free filter | Fab | https://www.fab.com/search?q=bar&price=free | Per-listing | Varies | ⚠ Fab's `price=free` URL param is inconsistent — always confirm the price shown says **Free** on the listing page |
| Starter content | UE Starter Content | Epic (installed with UE) | https://dev.epicgames.com/documentation/en-us/unreal-engine/creating-a-new-project-in-unreal-engine | Free (UE EULA) | `.uasset` | Create project → *Starter Content* checkbox. Basic materials, props, particles; not noir, but a working baseline |
| Marketplace | Fab "Free of the month" rotation | Fab | https://www.fab.com/search?q=free | Free while claimed | `.uasset` | ⚠ Limited-time giveaways — **claim to library to keep**. Rotates roughly monthly |

**Note on Quixel/Fab status:** quixel.com/megascans/free resolves (200), but Megascans is now served through Fab — expect the Fab licence, not the old Quixel licence, for anything downloaded after the migration.

---

## 8. Attribution & commercial-use checklist

1. **CC0 first.** Poly Haven, ambientCG, Quaternius, Kenney and Poly Pizza need nothing — start here for props, textures and street kits.
2. **CC-BY (Sketchfab) is fine** for a commercial game, but you must credit. Maintain `docs/CREDITS.txt` with: asset name, author, source URL, licence, link to the licence text.
3. **Avoid CC-BY-SA** in shipped content where a CC-BY or CC0 equivalent exists. One hit in this list — *Bar Shaker* — is SA; swap for a CC0 shaker.
4. **Never** CC-BY-NC, NC-ND, GPL-only, or anything re-uploaded from a commercial game. All such hits found during scanning were discarded (including a "Mafia II Definitive Edition" re-upload).
5. **Fab / Marketplace / Mixamo** run on their own EULAs, not Creative Commons: free to use in and shipped with your game, but don't resell the raw assets as assets. For Mixamo FBX used *outside* an Epic project, re-read the [Mixamo FAQ](https://www.mixamo.com) terms.
6. **Verify the *Downloadable* flag** on every Sketchfab page — free-to-view ≠ free-to-download.
7. **Re-check on download day.** Licences can be changed by the uploader; save a dated copy of the licence text per asset.

## 9. Recommended baseline (minimum set to grey-box the game)

| Need | Pick | Why |
| :--- | :--- | :--- |
| Bar room | *Pub interior* + *BarCounter - 8192px²* + *Wooden Back Bar Cabinet* | One blockout + two hero meshes |
| Glassware/bottles | *The ultimate glass pack* + *Wine Bottles* (CC0) | Every order type covered, no attribution |
| Seating | *bar_chair_round_01*, *metal_stool_01*, *Bar Stool and Table* | Three silhouettes, all free |
| Characters | *Universal Base Characters* + Mixamo clips | Rigged, retargetable, unlimited animations |
| Factions | *Boss Mafia Man*, *Character (Type Detective)*, *Man In Coat* | Reads instantly at silhouette level |
| Exterior | *Cyberpunk Game Kit* + *Modular Streets Pack* + *Alleyway* | Style match + modular roads + back alley |
| Police | *Police Car* (Sketchfab) + *concrete_road_barrier* + *service_pistol* | Cordon outside the bar |
| Materials | *Asphalt025C* + *Bricks097* + *WoodFloor064* + *Planks037A* | Wet street, alley walls, bar floor/counter |
| VFX | *Niagara Examples Pack* + Quixel Megascans free | Smoke, mist, wet surfaces |

**Total licence surface of the baseline:** 4 CC-BY entries to credit, everything else CC0 or platform-EULA.
