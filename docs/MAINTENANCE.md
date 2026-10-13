# Documentation maintenance routine
**[Русская версия этого файла →](MAINTENANCE.ru.md)**

How LAST CALL documentation is read and updated regularly. The user-facing rule: **documentation is read at the start of every work session and updated in the same change as the thing it describes.**

## 1. Language pairs

Every document exists twice and the pair must say the same thing:

| English | Русский |
| :--- | :--- |
| `README.md` | `README.ru.md` |
| `PHILOSOPHY.md` | `PHILOSOPHY.ru.md` |
| `ARCHITECTURE.md` | `ARCHITECTURE.ru.md` |
| `ROADMAP.md` | `ROADMAP.ru.md` |
| `CHANGELOG.md` | `CHANGELOG.ru.md` |
| `CONTRIBUTING.md` | `CONTRIBUTING.ru.md` |
| `CODE_OF_CONDUCT.md` | `CODE_OF_CONDUCT.ru.md` |
| `SECURITY.md` | `SECURITY.ru.md` |
| `VERSIONING.md` | `VERSIONING.ru.md` |
| `docs/GAME-DESIGN.md` | `docs/GAME-DESIGN.ru.md` |
| `docs/NARRATIVE.md` | `docs/NARRATIVE.ru.md` |
| `docs/ART-BIBLE.md` | `docs/ART-BIBLE.ru.md` |
| `docs/LANGUAGES.md` | `docs/LANGUAGES.ru.md` |
| `docs/FREE-3D-MODELS.md` | `docs/FREE-3D-MODELS.ru.md` |
| `docs/MAINTENANCE.md` | `docs/MAINTENANCE.ru.md` |

Rules:

1. **Both files, one change.** A change to one file without its twin is an incomplete change.
2. **Twin link at the top** of every file: `**[Русская версия этого файла →](FOO.ru.md)**` in English files and `**[English version of this file →](FOO.md)**` in Russian files (README-style header links also acceptable).
3. Numbers, dates, versions, code identifiers and URLs are never translated — they must match byte-for-byte between twins.
4. `CHANGELOG` twins keep the same section headings and versions; prose inside a section may be written natively in each language.

## 2. When to read

| Moment | What is read | Why |
| :--- | :--- | :--- |
| **Start of every session on this project** | `README` (current language) → `ROADMAP` → `CHANGELOG` (top section) | Restore context: what is decided, what is next, what just changed |
| **Before any design change** | `PHILOSOPHY` — always | It outranks everything, including memory of past decisions |
| **Before any code/tooling change** | `ARCHITECTURE` + the relevant `docs/*` document | Keep design and reality in sync |
| **Before a release** | `VERSIONING` + full `CHANGELOG` review | Correct headings, both languages, tags |
| **Weekly (or after a burst of work)** | Full pass, all files | Catch drift while it is small |

## 3. When to update

A document is updated **in the same change** as the thing it describes — never "later":

| Trigger | Update |
| :--- | :--- |
| A decision changes (stack, scope, tone, rule) | `PHILOSOPHY` first (with the owner's date), then documents that quoted it, then `CHANGELOG` |
| A system is added/renamed/removed | `ARCHITECTURE` table + `GAME-DESIGN` section |
| A milestone starts or finishes | `ROADMAP` status marker `[ ] → [~] → [x]` |
| Player-facing content changes | `GAME-DESIGN` / `NARRATIVE` / `ART-BIBLE` |
| Free assets are added or dropped | `FREE-3D-MODELS` (name, URL, license, format — all four required) |
| Anything user-visible | `CHANGELOG` section under `Unreleased`, then headed with the version at release |
| Language/engine research ages (> 3 months) | Refresh `LANGUAGES` — versions and dates move fast |
| A link breaks or a file is renamed | Fix both languages immediately |

`PHILOSOPHY` changes require the owner and get a date stamp inline; nobody else edits it.

## 4. Consistency checklist (run before every commit)

- [ ] Each edited `FOO.md` has its `FOO.ru.md` updated in the same change
- [ ] Twin link present at the top of both files
- [ ] All relative links resolve (no broken `](path)`)
- [ ] Identifiers, class names (`ULCPhaseManager`), versions (`b17`, `UE 5.8`), dates and URLs identical in both languages
- [ ] Status markers in `ROADMAP` match reality
- [ ] New decisions recorded in `PHILOSOPHY` (owner, dated)
- [ ] User-visible change added to `CHANGELOG` (+ twin) under `Unreleased`
- [ ] `FREE-3D-MODELS` entries still have working URLs and correct licenses

Quick link check from the repo root:

```bash
grep -oh '](\([^)#]*\)' *.md docs/*.md | sed 's/](//' | grep -v '^http' | sort -u | while read -r f; do
  [ -e "$f" ] || [ -e "docs/$f" ] || echo "MISSING: $f"
done
```

## 5. Review passes

1. **Session start** — read (§2), no writing required.
2. **Session end / change commit** — update (§3), run checklist (§4), commit docs with the change.
3. **Weekly full pass** — read every file end to end; fix drift, stale dates, dead links; note anything needing an owner decision in `CHANGELOG` as a question, not silently.
4. **Before any release** — `VERSIONING` + `CHANGELOG` bilingual review.

## 6. Rules that never bend

1. **PHILOSOPHY outranks everything** (§3 in that file includes agent memory).
2. **No undocumented decisions.** If it was decided in a chat, it is not decided until it is written, dated and in the right file.
3. **Both languages or nothing.** A one-language change blocks the commit.
4. **The roadmap is the truth about "when".** If `ROADMAP` says `[ ]`, it is not started — no matter what anyone remembers.
5. **Assets need licenses.** Nothing enters `FREE-3D-MODELS` without URL + license + format, and nothing unlicensed enters the game.
6. **The agent never idles on background work.** While a file downloads, an archive unpacks, an engine installs or any long job runs in the background, work continues on the next autonomous task (roadmap order: engine-independent cores, linters, documentation). When the background job reports, its result is processed and work continues. Waiting is not working.
