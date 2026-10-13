#!/usr/bin/env python3
"""One-shot documentation fixes: twin links + relative paths from docs/.

Kept in scripts/ for the record; the ongoing rules live in docs/MAINTENANCE.md.
"""
import pathlib

ROOT = pathlib.Path(__file__).resolve().parent.parent
ROOT_FILES = [
    "PHILOSOPHY", "ARCHITECTURE", "ROADMAP", "CHANGELOG", "CONTRIBUTING",
    "CODE_OF_CONDUCT", "SECURITY", "VERSIONING", "LICENSE", "README",
]


def twin_name(path: pathlib.Path) -> str:
    if path.name.endswith(".ru.md"):
        return path.name[: -len(".ru.md")] + ".md"
    return path.stem + ".ru.md"


def md_files():
    yield from sorted(ROOT.glob("*.md"))
    yield from sorted((ROOT / "docs").glob("*.md"))


def ensure_twin_link(path: pathlib.Path, text: str) -> str:
    twin = twin_name(path)
    if f"]({twin})" in text:
        return text
    line = path.name.endswith(".ru.md")
    link = (
        f"**[English version of this file →]({twin})**"
        if line
        else f"**[Русская версия этого файла →]({twin})**"
    )
    lines = text.splitlines(keepends=True)
    # Insert right after the H1 title (first line starting with '#').
    for i, current in enumerate(lines):
        if current.startswith("# "):
            insert_at = i + 1
            if insert_at < len(lines) and lines[insert_at].strip():
                lines.insert(insert_at, "\n" + link + "\n")
            else:
                lines.insert(insert_at, link + "\n")
            break
        if i == 0:
            lines.insert(0, link + "\n\n")
            break
    return "".join(lines)


def fix_docs_relative(text: str) -> str:
    # docs/*.md must reach root files with ../
    for name in ROOT_FILES:
        text = text.replace(f"]({name}.md)", f"](../{name}.md)")
        text = text.replace(f"]({name}.ru.md)", f"](../{name}.ru.md)")
    return text


changed = []
for path in md_files():
    original = path.read_text(encoding="utf-8")
    text = ensure_twin_link(path, original)
    if path.parent == ROOT / "docs":
        text = fix_docs_relative(text)
    if text != original:
        path.write_text(text, encoding="utf-8")
        changed.append(str(path.relative_to(ROOT)))

for c in changed:
    print(f"fixed: {c}")
print(f"{len(changed)} file(s) changed")
