# Versioning
**[Русская версия этого файла →](VERSIONING.ru.md)**

LAST CALL has two release channels and one version format per channel. The scheme mirrors [FlashAgent's](https://github.com/flashback7766/FlashAgent) two-tier idea — a build counter plus SemVer — adapted for a game with an editor and content.

## Formats

| Channel | Format | Example | Meaning |
|---|---|---|---|
| Dev / nightly | `d<N>` | `d42` | Local build counter. Only ever grows. Never shipped to players. |
| Beta | `b<N>` | `b17` | Build counter for playtest builds. Only ever grows. |
| Stable | `v<MAJOR>.<MINOR>.<PATCH>+b<N>` | `v0.3.0+b17` | A SemVer release cut from beta build `N`. |

- The `+b<N>` part uses SemVer build metadata and **takes precedence when ordering**, including between stable releases: `v0.4.0+b18 < v0.3.0+b19`.
- Leading `v` optional, `v1.2` means `v1.2.0`.
- Pre-1.0 rules while the game is in production: MINOR carries content updates and features, PATCH carries fixes. After 1.0: MINOR = content/patches, PATCH = hotfixes, MAJOR = something players rely on changes meaning.
- A dev build (`d<N>`) is never distributed outside the team; it has no ordering relation to beta.

## Ordering

One rule, used everywhere:

1. Versions with a build number compare by `N` first, including stables: `v1.0.0+b30 < v0.9.0+b31`.
2. Stables with the same build compare by `MAJOR.MINOR.PATCH`.
3. On a tied build, a stable is newer: `b17 < v0.3.0+b17 < b18`.
4. A legacy stable without `+b<N>` is newer than every numbered build; two unnumbered stables compare by `MAJOR.MINOR.PATCH`.

## What a build reports

- **In-game:** Settings → About shows the full version, the build channel, and the engine version.
- **Crash reports (local only):** the version is attached to the log file; nothing is uploaded (PHILOSOPHY §3: no telemetry).
- **Save compatibility:** saves carry `saveVersion`; a save from a newer build is refused, older saves are migrated forward by `ULCSaveSystem` and the migration is logged. Migration never happens on load of a *newer* save — no silent data loss.

## Making a release

Beta build (the usual case). First write the `CHANGELOG.md` section (and its `.ru.md` twin), headed with the exact version:

```markdown
## b18 — short title
```

Then bump `BuildVersion` in the project settings, commit the changelog pair together with the bump as `Release b18: short title`, and tag that commit `b18`. A published tag is never moved; the script (once `tools/lc-release` exists) stops if the tag exists, the changelog has no section for it, or other changes are uncommitted.

Stable release, cut from the newest beta build. The changelog section comes first, headed with the full stable version:

```markdown
## v0.3.0+b18 — short title
```

Tag the same commit `v0.3.0+b18`.

### Choosing MAJOR.MINOR.PATCH

- **PATCH**: fixes only — crashes, balance bugs, typos in both languages.
- **MINOR**: new content or features; existing saves keep working (migrated).
- **MAJOR**: something players rely on stops working as before (save format break, control scheme change).

Beta build numbers stay the same across stable releases: the next beta after `v0.3.0+b18` is `b19` or higher.

## Channels

| Channel | Audience | Contents |
|---|---|---|
| `dev` | team | `d<N>` builds, may not boot |
| `beta` | playtesters | `b<N>` builds and stables newer than them |
| `stable` | players | stable releases only |

The updater (store builds) only ever moves forward on its channel, by the ordering above.
