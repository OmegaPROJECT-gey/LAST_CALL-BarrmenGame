# Security policy

## Supported versions

Fixes go into the next release on the channel they belong to; only the latest builds get them.

| Version | Supported |
| --- | :---: |
| Latest stable (`vX.Y.Z+bN`) | :white_check_mark: |
| Latest beta (`bNNN`, playtest channel) | :white_check_mark: |
| Older builds | :x: |

Save compatibility rules are in [VERSIONING.md](VERSIONING.md): a save from a newer build is refused rather than loaded, so an old build can never corrupt a new save.

## What counts as a security issue here

A single-player game with no servers and no telemetry (PHILOSOPHY §3) has a small attack surface, but these are security issues:

- **Save corruption or arbitrary file write** from crafted save files, imported data or `Data/` JSON.
- **Code execution** through any content path: dialogue files, mods, localization files, image/audio import.
- **Co-op abuse** that executes code, corrupts another player's save, or lets a client force a host into actions outside the game rules.
- **Privacy**: anything that would send player data somewhere without explicit opt-in — there is no telemetry, so any outbound traffic at all is a bug worth reporting.

Not security issues (use a normal bug report): crashes from gameplay, balance problems, translation mistakes, mod incompatibilities.

## Reporting a vulnerability

1. **Do not report it publicly** (issues, discussions, social media) — it would expose players before a fix exists.
2. Open a [GitHub security advisory](https://github.com/flashback7766/GameBarmenSimulator/security/advisories/new) on this repository, or contact the maintainer privately (contact link to be added here when the project publishes one).
3. Give the steps to reproduce: OS, build version from [VERSIONING.md](VERSIONING.md), save file if relevant (strip personal data), and the exact input.

You get an answer within 7 days; the fix and its disclosure are agreed with you. Reported and confirmed issues are credited in `CHANGELOG.md` unless you ask not to be.
