# Programming languages & engines research — LAST CALL

**Research date:** October 2026
**Project profile:** Realistic, story-driven 3D simulation/action RPG for PC. Noir bar-management (fluid-mixing physics), stealth/action night missions, deep NPC dialogue, 2–4 player co-op. Visual target: photorealism with noir elements (GI/raytraced lighting, wet surfaces, volumetric fog).

**[Русская версия этого файла →](LANGUAGES.ru.md)**

---

## TL;DR

**Primary stack: C++ with Unreal Engine 5 (current stable: UE 5.8, released June 2026).** It is the only mainstream engine that ships the exact rendering stack this project needs out of the box (Nanite + Lumen + MegaLights + Lumen Lite + volumetric fog + fluid/VFX simulation), has production-grade co-op networking, and has the largest hiring pool. The new languages (Rust/Bevy, Zig, Odin) are genuinely exciting in 2026 but none of them can deliver "photoreal noir 3D + co-op netcode + content pipeline" without years of engine-building.

**Supporting stack:** Python for editor/asset-pipeline scripting (UE has first-class Python API), Rust or Zig (your pick — both are excellent) for standalone CLI build/asset tools where single-binary distribution and speed matter. No Carbon. No Swift.

---

## Comparison Table

| Language / Stack | Maturity for 3D games | Ecosystem | Performance | Hiring / dev speed | Verdict for THIS project |
|---|---|---|---|---|---|
| **C++ (Unreal Engine 5.8)** | **Excellent** — industry standard, AAA-proven, full toolset (Nanite/Lumen/MegaLights/Chaos/Niagara) | **Excellent** — huge marketplace (Fab), docs, tutorials, plugins for dialogue/water/AI | Excellent (native, SIMD, full GPU access; console-grade) | Large talent pool; slower iteration (compile/cook), Blueprint mitigates | ✅ **PRIMARY STACK** — the only realistic option for the visual + co-op target |
| **Rust (Bevy 0.20 / wgpu)** | Good-and-improving, but **pre-1.0** — Bevy 0.20 (Oct 2026) has a real path tracer (Solari) + mesh shaders, but no shipped AA 3D game | Good and fast-growing (crates.io, Bevy Assets), but editor still in development | Excellent (zero-cost abstractions, no GC; comparable to C++) | Small hiring pool; steep for non-systems programmers; engine-dev risk | ❌ Not for this game. ✅ Maybe for **CLI pipeline tools** |
| **Zig (0.17.0, Oct 2026)** | Niche — Mach engine v0.4-dev; zig-gamedev ecosystem; a few shipped indie games (e.g. *Kandria*) | Small but healthy; **superb C/C++ interop** (can consume UE/C++ libs) | Excellent (C-level, no hidden allocs) | Very small hiring pool; language still 0.x (breaking changes each release) | ❌ Not for this game. ✅ Strong candidate for **CLI tools** (single static binary, fast builds) |
| **Odin (pre-1.0; 1.0 announced for Jan 2027)** | Niche but real — shipped Steam games (Terrafactor, DUMMS!, CAT & ONION…), hot reload is best-in-class | Small; official bindings (raylib, sokol, SDL2); no engine/editor | Excellent (C-level, data-oriented, array programming) | Tiny hiring pool; solo-dev language | ❌ Not for this game. ✅ Fun for tools/prototypes if team already knows it |
| **C# (Unity 6.x)** | Good, but wrong tool here — Unity's 3D photorealism ceiling is far below UE5; 2026 pricing changes still souring trust | Excellent asset store, huge community | Good (IL2CPP), but rendering bottleneck, not CPU | Very large pool; fastest 2D/indie iteration | ❌ Rendering target not achievable without building a renderer |
| **C# / GDScript (Godot 4.6)** | Stylized/indie 3D only — SDFGI GI, no hardware-RT GI, weak photorealism, console support via third-party (W4) | Excellent for indie; rapid growth; MIT-licensed | Good for its class; far from UE5 | Fastest iteration of all; easiest language (GDScript) | ❌ Only for spin-offs/2D, not the main game |
| **Carbon** | **Experimental only** — team explicitly advises against production use in 2026 | Almost none for games | N/A (no production toolchain) | N/A | ❌ Do not consider |
| **Python** | N/A (not for runtime gameplay) | Ubiquitous for tooling; **UE Editor Python API** built-in | Slow | Fastest for glue/automation | ✅ **YES — pipeline/editor scripting** |

---

## 1. C++ with Unreal Engine 5 — the chosen stack

### Current state (2026)

- **UE 5.8** is the current release, launched at **State of Unreal 2026** (~June 18, 2026) ([Epic](https://www.unrealengine.com/news/unreal-engine-5-8-is-now-available), [CGChannel](https://www.cgchannel.com/2026/06/see-5-key-features-for-cg-artists-in-unreal-engine-5-8/)).
  Headline features:
  - **Lumen Lite** — a faster, "medium-quality" mode of Lumen GI designed to preserve visual impact while targeting **60 fps** (ideal for our action/stealth missions).
  - **MegaLights** now **production-ready** — new direct-lighting path for hundreds of dynamic, shadowed lights (huge for a noir bar with many lamps/neon signs). Introduced experimentally in 5.5 (up to ~50% faster on RTX 4080 @4K in NVIDIA tests).
  - **Mesh-based terrain system**, **PSO precaching** improvements (fewer shader-compile hitches on first play).
  - **Chaos Fluids, Chaos Destruction, Chaos Hair** simulation updates ([CGChannel](https://www.cgchannel.com/2026/06/see-5-key-features-for-cg-artists-in-unreal-engine-5-8/)).
- **Release cadence:** 5.5 (Nov 2024, MegaLights) → 5.6 (June 2025, Nanite skeletal characters/new MetaHuman creator, **Water "Advanced" plugin** with shallow-water rivers) → 5.7 (late 2025, wide performance pass, [tomlooman](https://tomlooman.com/unreal-engine-5-7-performance-highlights/)) → 5.8 (June 2026). Roughly two feature releases per year.
- **Unreal Engine 6 is already announced** (Unreal Fest Seoul): it will merge Unreal Engine and UEFN into one engine and integrate AI models for tedious work; **early access ~end of 2027, full release ~12–18 months later** ([VGC](https://www.videogameschronicle.com/news/epic-reveals-unreal-engine-6-is-integrating-ai-models-so-developers-can-reduce-tedious-work/), [Epic road map](https://www.unrealengine.com/news/the-road-to-ue-6)). **Implication:** build on UE 5.8 now; plan an engine upgrade path for UE6 mid-project/later, do not wait for it.

### Why it fits this project

- **Rendering (noir photorealism):** Nanite (virtualized micropolygon geometry — no manual LODs), Lumen GI/reflections (hardware Lumen in 5.8 reduces "boiling" artifacts), MegaLights (many dynamic lights), Virtual Shadow Maps, Substrate materials, local **volumetric fog** — exactly the wet-asphalt, smoky-bar, neon-noir look we want, without writing a renderer.
- **Fluid/physics:** Chaos physics + Niagara Fluids (see §7) covers pouring/splashing liquids and destruction; UE 5.6+ Water plugin for larger bodies.
- **Co-op networking (2–4 players):** mature client–server replication built into the engine ([Networking Overview](https://dev.epicgames.com/documentation/unreal-engine/networking-overview-for-unreal-engine)). For 2–4 player co-op the recommended 2026 pattern is a **listen server + Epic Online Services (NAT relay)** — no dedicated-server cost, works behind consumer routers ([overview + EOS](https://www.strayspark.studio/blog/ue5-multiplayer-2026-replication-eos)). Replication, prediction, and relevancy are decades-old, battle-tested code paths.
- **Workflow:** Blueprint (visual scripting, fast iteration for designers/writers) + C++ (systems, netcode, performance-critical code). Hot iteration on gameplay via **Live Coding** (Ctrl+Alt+F11) — patches DLLs while the editor runs ([docs](https://dev.epicgames.com/documentation/unreal-engine/using-live-coding-to-recompile-unreal-engine-applications-at-runtime)).
- **Dialogue/NPC AI tooling:** **StateTree** (modern hierarchical state machine, now preferred over Behavior Trees in 2026 — [quick start](https://dev.epicgames.com/documentation/unreal-engine/statetree-quick-start-guide), [migration analyses](https://www.strayspark.studio/blog/state-tree-vs-behavior-tree-ue5-7-migration-2026)), Behavior Trees (legacy, still supported), **MassAI** (crowds), **EQS** (environment queries), Perception system (sight/hearing for stealth), Gameplay Tags + DataTables for data-driven dialogue content. LLM-NPC middleware (e.g. Convai) also ships UE5 plugins if we ever want dynamic NPCs ([Convai](https://convai.com/blog/how-to-make-ai-npcs-act-on-your-commands-in-unreal-engine-5-with-convai)).
- **Content pipeline:** Datasmith/Fab importers, Control Rig, MetaHuman, sequencer cinematics for story scenes, built-in Localization Dashboard for dialogue.

### Honest pain points

- **Compile & cook times.** Full C++ rebuilds on big projects are slow; the cook (asset packaging) does a lot of work per invocation on large projects ([Unreal Fest cook deep-dive](https://www.youtube.com/watch?v=6L4Mz4FtMuY)). Mitigations: Unity-style small headers, Live Coding for iterative edits, source-build vs launcher-build tradeoffs.
- **Hot Reload legacy is unreliable** (Blueprint corruption reports); use **Live Coding** instead, and expect occasional full restarts ([community wiki](https://unrealcommunity.wiki/live-compiling-in-unreal-projects-tp14jcgs), [forums](https://forums.unrealengine.com/t/slow-c-compile-time-for-trivial-changes-hot-reload/407579)).
- **Learning curve** is real: C++ templates, UObject/Blueprint GC model, replication concepts, cook/package pipeline.
- **Performance discipline required:** Lumen/Nanite are heavy; the "UE5 is unoptimized" meme is mostly misconfiguration ([discussion](https://www.reddit.com/r/unrealengine/comments/1rkqz4v/saying_unreal_engine_5_is_not_optimized_is_not_even_fair/)) — but our team must own profiling (Unreal Insights, PSO caching, Lumen Lite for 60 fps targets).
- **Royalty:** free until $1M revenue, then 5% royalty (standard Epic terms) — fine for us, but note it vs MIT engines.

**Bottom line:** UE 5.8 C++ is confirmed as the correct and lowest-risk choice. Nothing else in 2026 ships this rendering + netcode + tooling package.

---

## 2. Rust (Bevy, wgpu/winit, bindings)

### Current state (2026)

- **Bevy 0.20** released **October 8, 2026** (227 contributors, 817 PRs) ([release notes](https://bevy.org/news/bevy-0-20/)). Recent cadence: 0.17 (Sep 2025) → 0.18 (Jan 2026) → 0.19 (Jun 2026) → 0.20 (Oct 2026). Bevy turned **6 years old** in Aug 2026 and is backed by the **Bevy Foundation** (US 501(c)(3) nonprofit).
- **3D capability is now seriously impressive for an indie engine:** **Solari**, a real-time **path-traced renderer** (ReSTIR, world caching) with **DLSS-RR 4.5** support via `dlss_wgpu`; **mesh shaders**; **WESL** standardized shader language with LSP tooling (wgsl-analyzer); BSN scene system; Feathers UI toolkit (editor UI is being built on it — the **official editor is still in development**).
- **Custom Rust + wgpu/winit** remains a viable route for rendering-focused projects; `arewegameyet.rs` tracks the ecosystem. **Fyrox** is the most "production-ready" general Rust 3D engine ([fyrox.rs](https://fyrox.rs/)), still far less capable than UE5.
- **Bindings:** **godot-rust/gdext** is healthy (v0.5, March 2026 dev update — performance work, safeguard levels) ([godot-rust](https://godot-rust.github.io/dev/march-2026-update/)). There is **no usable official or community Rust binding for Unreal Engine 5** — writing gameplay Rust inside UE is not a real option in 2026.

### Pros / cons (honestly)

- ✅ Memory safety without GC, excellent performance, best-in-class build tooling for **CLI tools**, fearless concurrency for server-side simulation (co-op server logic would be lovely in Rust).
- ❌ **Bevy is pre-1.0**: breaking API changes every ~3 months; every release requires migration work ([migration guides](https://bevy.org/learn/migration-guides/0-19-to-0-20/)).
- ❌ No mature editor, no equivalent of Nanite/Lumen/MegaLights/Chaos/Niagara — Solari path tracing is real but it's a research-grade renderer, not a shipped-game feature set with LOD, streaming and art tooling.
- ❌ **No netcode framework** comparable to UE replication (lightyear/bevy_replicon exist but are young).
- ❌ Borrow checker + ECS + reflection = high friction; hiring pool for "Rust gamedev" is tiny. Community warning: "don't get sucked into engine dev and never make your game" ([r/bevy](https://www.reddit.com/r/bevy/comments/1n0o8m5/current_state_of_bevy_for_professional_game/)).

**Verdict:** Not for the game. Plausible for **standalone tools** (see §8) or future side-projects.

---

## 3. Zig

### Current state (2026)

- **Zig 0.17.0** released **October 1, 2026**; 0.16.0 (Apr 13, 2026); 0.15.x (Aug–Oct 2025). Master is 0.18.0-dev. Source now hosted on Codeberg ([ziglang.org/download](https://ziglang.org/download/index.html)).
- **Game ecosystem:** **Mach** engine (in development since 2021) is at **v0.4-dev** — explicitly "we're not there yet" as a comprehensive engine with editor ([machengine.org](https://machengine.org/)). **zig-gamedev** provides libs/samples (Jolt physics bindings, AI, graphics) ([github.com/zig-gamedev](https://github.com/zig-gamedev)). Shipped commercial example: **Kandria** (isometric action RPG, one of the first Zig games); indie devs report 100k+ lines of production Zig game code ([dev diary](https://www.youtube.com/watch?v=HXpUShkr2VQ)).
- **Why people love it:** **C interop is the killer feature** — `@cImport`/translate-c lets you call any C/C++ library directly, which means Zig can talk to existing engine/native codebases without binding-generator hell. Comptime metaprogramming, fast single-file builds, trivial cross-compilation (one binary, every OS), no build-system zoo.

### Pros / cons

- ✅ Best-in-class "glue" language for C/C++ ecosystems; fantastic for **asset-pipeline CLI tools** (fast startup, static binaries, cross-compile).
- ❌ Still **0.x** — the language churns (breaking changes between releases; engines like Mach pin specific nightly builds, a real maintenance hazard).
- ❌ Tiny hiring pool; no game engine with editor anywhere near UE5/Unity maturity.
- ❌ No GC/runtime means more manual memory design work in gameplay code.

**Verdict:** Not for the game. Excellent candidate for **pipeline/CLI tooling** if the team prefers it over Rust.

---

## 4. Odin

### Current state (2026)

- **Odin 1.0 was announced** by creator gingerBill (video, late 2025) — "a decade in the making", **shipping January 2027** ([announcement](https://www.youtube.com/watch?v=dLPAqXi9In0), [discussion](https://news.ycombinator.com/item?id=48813850)). Until then it's stable-enough 0.x used in production by individuals.
- **Shipped games (from the official showcase)**: *Terrafactor* (released), *DUMMS!*, *Solar Storm*, *2Deez*, *Relative Frame*, *Volt Deck*, *CAT & ONION* — mostly built with **raylib/sokol bindings**; scopes are small/indie ([odin-lang.org/games](https://odin-lang.org/games/)).
- **Standout features:** **hot reload of gameplay code** (executable + shared-library pattern keeps game memory while swapping code — the canonical writeup is Karl Zylinski's [Hot Reload Gameplay Code](https://zylinski.se/posts/hot-reload-gameplay-code/)), **array programming** (SIMD-friendly builtin ops), bit sets, slices, no hidden allocations/ASTs, official vendored bindings (SDL2, raylib, Vulkan, etc.).

### Pros / cons

- ✅ Lowest-friction systems language of the bunch; reads like clean C with data-oriented design baked in; hot reload is genuinely faster iteration than UE Live Coding.
- ❌ **No engine, no editor, no netcode, no asset pipeline** — everything must be built or glued from libraries.
- ❌ Smallest hiring pool of all options; pre-1.0.
- ❌ None of the shipped games approach photoreal 3D.

**Verdict:** Not for the game. Keep in the back pocket for tools if someone on the team already loves it.

---

## 5. Other options

### C# with Unity (6.x, 2026)
- Still the biggest indie/professional ecosystem, but **Unity announced further pricing/packaging changes in 2026** ([unity.com pricing updates](https://unity.com/products/pricing-updates)) and post-2023-runaway-fee trust damage lingers ([discussion](https://www.reddit.com/r/Unity3D/comments/1tvo9bb/is_unity_still_your_first_choice_for_indie/)).
- Surveys at GDC 2026 put **Unreal ahead of Unity in studio mindshare (~42% vs ~30%)** ([survey coverage](https://shattered.io/unreal-engine-6-unity-market-share-2026/) — treat as indicative, not gospel).
- **Rendering:** Unity cannot reach UE5-class photorealism (no Nanite/Lumen equivalent); HDRP helps but caps out well below our target.
- **Verdict:** Wrong engine for this project. Excellent for mobile/2D/live-ops — relevant only if we ship a mobile spin-off.

### C#/GDScript with Godot 4.6
- Godot is thriving as the open-source indie engine (4.6 brings SSR rewrite, lightmapper and Vulkan improvements — [technical overview](https://www.strayspark.studio/blog/godot-46-rendering-deep-dive-ssr-lightmapper-performance)).
- **3D realism ceiling:** SDFGI is a decent real-time GI but is soft/inaccurate at extremes ([known issues](https://github.com/godotengine/godot/issues/86833)); **no hardware ray tracing / no Lumen-class GI**; community consensus: great for indie/stylized 3D, not photoreal AAA-style visuals ([forum](https://forum.godotengine.org/t/does-godot-really-have-the-power-to-create-photorealistic-games/85467), [analysis](https://ziva.sh/blogs/godot-3d)). Console support requires W4 Games middleware.
- **Verdict:** Not for the main game. Great for **2D spin-offs, mods, or narrative side-products** where C#/GDScript speed matters.

### Carbon (Google)
- Still **experimental in 2026**; the project itself advises against production use; a compiler/explorer exists but there is no game ecosystem, no standard library maturity story for games ([carbon-lang.dev](https://docs.carbon-lang.dev/), [2026 status overview](https://www.programming-helper.com/tech/carbon-programming-language-2026-google-experimental-cpp-successor)).
- **Verdict:** Ignore for this decade.

### Swift
- Irrelevant for a PC AAA-visual game (macOS/iOS-centric toolchain, no game engine ecosystem). Skipping per brief.

---

## 6. Engine-level context: the "new languages" vs this project's needs

What our project needs, ranked by risk if built in-house:

1. **Photoreal rendering stack** (GI, reflections, RT, fog, wet materials, upscaling) — years of renderer work. UE5 gives it day one.
2. **Co-op netcode** (replication, lag compensation, NAT traversal, matchmaking/session) — UE replication + EOS gives it day one.
3. **Content pipeline** (meshes, animation, cinematics, localization, cook/packaging) — UE gives it day one.
4. **Fluid mixing + physics** — UE Chaos/Niagara covers most of it (see §7).
5. **Dialogue & AI behavior** — UE StateTree/Behavior Trees/DataTables (see §7).

Bevy/Zig/Odin would leave items 1–4 as multi-year engine projects. Rust's safety and Zig/Odin's ergonomics are real, but they don't move the critical path for *this* game.

---

## 7. Game-specific subsystem notes (UE5)

### Fluid mixing / liquid simulation options in UE5

| Option | What it is | Fit for a cocktail-mixing bar game |
|---|---|---|
| **Niagara Fluids** | Grid-based (Eulerian) sim with **FLIP** for liquids, plus fire/smoke/gas templates ([Epic docs](https://dev.epicgames.com/documentation/unreal-engine/fluid-simulation-in-unreal-engine---overview), [Niagara Fluids](https://dev.epicgames.com/documentation/unreal-engine/niagara-fluids-in-unreal-engine?lang=en-US)) | Good for **hero pouring/splash VFX** (bottles, glasses). GPU-heavy — users report ~80% GPU load on an RTX 3080 for a 3D FLIP sim ([community report](https://www.reddit.com/r/unrealengine/comments/zbulm6/absolute_noob_niagara_fluids/)). Use small sim domains, bake where possible. |
| **Chaos Fluids** | Epic's Chaos-based fluid system; roadmap shows continued investment (5.7 baked-river improvements; 5.8 sim updates) ([roadmap](https://portal.productboard.com/epicgames/1-unreal-engine-public-roadmap/c/2215-chaos-fluids)) | Better for **larger/scenario** sims; not a per-drink runtime system. |
| **Water plugin (5.6+ "Advanced")** | Rivers/lakes/ocean with shallow-water river actor (Unreal Fest 2025) | Only if the noir city has outdoor water (rain puddles/rivers). Overkill for glasses. |
| **Custom lightweight approach (recommended core)** | Height-field/vertex-deformation liquid mesh in the glass + refraction shader + Niagara particle splashes + metaball "pour stream" mesh | **This is what shipped bar-tending games actually do.** Cheap, replicates trivially (sync a few floats), art-directable (noir look!). |
| **Third-party** | **Fluid Flux** (Fab) — popular real-time water sim plugin with async Niagara fluids and flow-map baking ([Fab listing](https://www.fab.com/listings/196c70cd-1283-4249-bf6b-c3019d1cbe11?lang=en-US), [forum recommendation](https://forums.unrealengine.com/t/which-fluid-simulation-should-i-choose/1709509)); older "easy-FluidSurface"-style surface plugins | Evaluate Fluid Flux if we need convincing water/puddle FX beyond Niagara templates. |

**Recommendation:** gameplay-facing mixing = **custom lightweight liquid representation** (deterministic, server-authoritative, replicatable), with **Niagara Fluids** for localized hero VFX. Full FLIP per glass is neither performant nor network-friendly.

### Dialogue systems

- Data-driven approach in pure UE: **DataTables (CSV/JSON) + Gameplay Tags + a custom `UDialogueComponent`**, dialogue trees authored by writers in external sheets; Localization Dashboard for shipping translations.
- **StateTree** can drive conversation state machines (dialogue as states with transitions) — increasingly the community default over Behavior Trees ([StateTree quick start](https://dev.epicgames.com/documentation/unreal-engine/statetree-quick-start-guide), [dialogue-in-StateTree tutorials](https://www.youtube.com/watch?v=76Ii2Z5AQ6w)).
- Third-party dialogue plugins on Fab exist (Behavior-Tree-based conversation systems, subtitle/voice managers); evaluate vs building in-house depending on writer workflow needs.
- For **co-op dialogue**: drive dialogue state on the server, replicate current node + choices (small data), like any replicated interaction.

### AI / behavior tools (stealth/action night missions)

- **StateTree** (modern, preferred in 2026) + **EQS** (position queries: cover, patrol points) + **AI Perception** (sight cones, hearing stimuli — core of stealth) + **NavMesh**.
- **MassAI** for crowds/background patrons if needed.
- **Behavior Trees** remain fully supported with the largest tutorial corpus.
- Optional: LLM NPC middleware (Convai etc.) — not needed for scripted noir, but the plugin ecosystem exists.

---

## 8. Final recommendation

### Primary stack (the game)

- **Unreal Engine 5.8 (C++ + Blueprint)**, PC-first, DX12/Vulkan.
  - Gameplay systems, netcode, performance-critical code → **C++**.
  - Designer/writer-facing iteration (dialogue flow, level scripting, VFX tweaking) → **Blueprint**.
  - Rendering defaults for our look: **Lumen (with Lumen Lite for perf scenes) + Hardware Lumen where affordable, MegaLights for the bar interiors, Nanite everywhere, VSM shadows, local volumetric fog, Substrate materials for wet surfaces, TSR/DLSS/FSR upscaling, PSO precaching on**.
  - Multiplayer: **listen server + EOS** for 2–4 player co-op (no dedicated-server cost).
  - Watch the **UE6 timeline** (early access ~end 2027): budget an engine-version migration later in the project; don't bet features on UE6 now.

### Tools / pipeline (where the new languages earn their place)

| Need | Recommended language | Why |
|---|---|---|
| Editor automation, asset post-processing, data validation, build orchestration | **Python** | Built into UE (Python editor scripting API); every tech artist knows it; fastest glue. |
| Standalone CLI tools (batch asset conversion, texture/mesh pipelines, dialogue-sheet compilers, CI helpers) | **Rust** (or **Zig** if the team prefers its C interop and single-binary ergonomics) | Single static binary, fast, no runtime deps, great CI story. Zig is uniquely good if tools must call existing C/C++ libs. Pick **one** to keep the toolchain uniform. |
| Shader work | HLSL (UE) | Native to the engine. |
| Data authoring (dialogue, recipes, items) | CSV/JSON/SQLite + Python validators | Designers edit plain data; pipeline validates before cook. |

### Explicit non-choices

- **Rust/Bevy for the game:** rejected — pre-1.0 editor/renderer, no netcode equivalent, hiring risk, and it can't hit the visual bar without multi-year engine work. (Revisit for future projects; Solari is genuinely promising.)
- **Zig/Odin for the game:** rejected — no engine/editor ecosystem; 0.x churn; tiny talent pool. Keep them on the shortlist for **tools**.
- **Carbon:** rejected — experimental, explicitly not production-ready.
- **Unity/Godot/C#:** rejected for the main game (rendering ceiling, pricing/trust); Godot fine for future 2D spin-offs.
- **Swift:** irrelevant.

---

## Sources (key claims)

**Unreal Engine**
- UE 5.8 announcement — https://www.unrealengine.com/news/unreal-engine-5-8-is-now-available
- UE 5.8 features for CG artists (MegaLights production-ready, Lumen Lite, Chaos sim updates) — https://www.cgchannel.com/2026/06/see-5-key-features-for-cg-artists-in-unreal-engine-5-8/
- UE 5.8 performance highlights — https://tomlooman.com/unreal-engine-5-8-performance-highlights/
- MegaLights docs — https://dev.epicgames.com/documentation/unreal-engine/megalights-in-unreal-engine?lang=en-US
- UE 5.7 performance — https://tomlooman.com/unreal-engine-5-7-performance-highlights/
- UE6 roadmap (UE + UEFN unification) — https://www.unrealengine.com/news/the-road-to-ue-6 ; timing/AI claims — https://www.videogameschronicle.com/news/epic-reveals-unreal-engine-6-is-integrating-ai-models-so-developers-can-reduce-tedious-work/
- Networking overview — https://dev.epicgames.com/documentation/unreal-engine/networking-overview-for-unreal-engine ; co-op listen-server + EOS guidance — https://www.strayspark.studio/blog/ue5-multiplayer-2026-replication-eos
- Live Coding docs — https://dev.epicgames.com/documentation/unreal-engine/using-live-coding-to-recompile-unreal-engine-applications-at-runtime ; Hot Reload issues — https://unrealcommunity.wiki/live-compiling-in-unreal-projects-tp14jcgs
- StateTree quick start — https://dev.epicgames.com/documentation/unreal-engine/statetree-quick-start-guide ; StateTree vs Behavior Tree (2026) — https://www.strayspark.studio/blog/state-tree-vs-behavior-tree-ue5-7-migration-2026
- Fluid simulation overview (FLIP liquids) — https://dev.epicgames.com/documentation/unreal-engine/fluid-simulation-in-unreal-engine---overview ; Niagara Fluids — https://dev.epicgames.com/documentation/unreal-engine/niagara-fluids-in-unreal-engine?lang=en-US ; Chaos Fluids roadmap — https://portal.productboard.com/epicgames/1-unreal-engine-public-roadmap/c/2215-chaos-fluids
- Fluid Flux (third-party water) — https://www.fab.com/listings/196c70cd-1283-4249-bf6b-c3019d1cbe11?lang=en-US

**Rust / Bevy**
- Bevy 0.20 (Oct 8, 2026; Solari path tracing, DLSS-RR 4.5, mesh shaders, WESL) — https://bevy.org/news/bevy-0-20/
- Bevy release history / foundation — https://bevy.org/news/ ; 6th birthday — https://bevy.org/news/bevys-sixth-birthday/
- Professional-dev reality check — https://www.reddit.com/r/bevy/comments/1n0o8m5/current_state_of_bevy_for_professional_game/
- godot-rust gdext (Rust ↔ Godot 4) — https://godot-rust.github.io/ ; March 2026 update — https://godot-rust.github.io/dev/march-2026-update/
- Fyrox (Rust 3D engine) — https://fyrox.rs/

**Zig**
- Release page (0.17.0 Oct 1, 2026; 0.16.0 Apr 13, 2026) — https://ziglang.org/download/index.html
- Mach engine status (v0.4-dev, "not there yet") — https://machengine.org/ ; v0.3 release — https://devlog.hexops.org/2024/mach-v0.3-released/
- zig-gamedev — https://github.com/zig-gamedev ; long-form Zig gamedev experience — https://www.youtube.com/watch?v=HXpUShkr2VQ

**Odin**
- Games showcase (Terrafactor, DUMMS!, CAT & ONION, etc.) — https://odin-lang.org/games/
- Odin 1.0 announcement (January 2027) — https://www.youtube.com/watch?v=dLPAqXi9In0 ; HN discussion — https://news.ycombinator.com/item?id=48813850
- Hot reload gameplay code — https://zylinski.se/posts/hot-reload-gameplay-code/ ; no-engine gamedev with Odin + Raylib — https://zylinski.se/posts/no-engine-gamedev-using-odin-and-raylib/

**Godot / Unity / Carbon**
- Godot photorealism feasibility discussion — https://forum.godotengine.org/t/does-godot-really-have-the-power-to-create-photorealistic-games/85467 ; 3D limits — https://ziva.sh/blogs/godot-3d ; SDFGI docs — https://docs.godotengine.org/en/stable/tutorials/3d/global_illumination/using_sdfgi.html ; SDFGI accuracy issues — https://github.com/godotengine/godot/issues/86833
- Unity 2026 pricing changes — https://unity.com/products/pricing-updates
- Carbon docs (experimental status) — https://docs.carbon-lang.dev/ ; 2026 status — https://www.programming-helper.com/tech/carbon-programming-language-2026-google-experimental-cpp-successor
