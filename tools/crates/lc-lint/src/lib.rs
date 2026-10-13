//! Shared helpers for the LAST CALL linters.
//!
//! Three binaries sit on top of this: `lc-data-lint` (design data against
//! JSON Schemas), `lc-dialogue-lint` (dialogue graph + EN/RU parity) and
//! `lc-docs-check` (bilingual documentation pairs + link integrity).
//!
//! License: MIT (LICENSE in the repository root).

use std::path::{Path, PathBuf};

use regex::Regex;

/// Walk up from the current directory until the repository root is found
/// (marked by `LastCall.uproject`, falling back to `.git`).
pub fn repo_root() -> PathBuf {
    let mut dir = std::env::current_dir().expect("current dir");
    loop {
        if dir.join("LastCall.uproject").exists() || dir.join(".git").exists() {
            return dir;
        }
        if !dir.pop() {
            panic!("repository root not found above {:?}", std::env::current_dir());
        }
    }
}

/// Remove fenced code blocks and inline code spans so that link extraction
/// and pair checks never trip over documentation examples (`docs/MAINTENANCE.md`
/// is full of them).
pub fn strip_code(text: &str) -> String {
    let fenced = Regex::new(r"(?ms)```.*?```").expect("fenced regex");
    let without_fenced = fenced.replace_all(text, "");
    let inline = Regex::new(r"`[^`\n]*`").expect("inline regex");
    inline.replace_all(&without_fenced, "").into_owned()
}

/// Extract markdown link targets (`](target)`) from code-stripped text.
pub fn extract_links(text: &str) -> Vec<String> {
    let re = Regex::new(r"\]\(([^)\s]+)\)").expect("link regex");
    re.captures_iter(text)
        .map(|c| c[1].to_string())
        .filter(|t| !t.is_empty())
        .collect()
}

/// Every markdown file in the repo (root and `docs/`), sorted, relative to root.
pub fn markdown_files(root: &Path) -> Vec<PathBuf> {
    let mut files = Vec::new();
    for entry in std::fs::read_dir(root).expect("read repo root") {
        let path = entry.expect("dir entry").path();
        if path.is_file() && path.extension().is_some_and(|e| e == "md") {
            files.push(path);
        }
    }
    let docs = root.join("docs");
    if let Ok(entries) = std::fs::read_dir(&docs) {
        for entry in entries {
            let path = entry.expect("dir entry").path();
            if path.is_file() && path.extension().is_some_and(|e| e == "md") {
                files.push(path);
            }
        }
    }
    files.sort();
    files
}

/// The language twin of a markdown path:
/// `README.md` -> `README.ru.md`, `docs/FOO.ru.md` -> `docs/FOO.md`.
pub fn twin_of(path: &Path) -> PathBuf {
    let name = path.file_name().expect("file name").to_string_lossy().into_owned();
    let dir = path.parent().map(Path::to_path_buf).unwrap_or_default();
    if let Some(stem) = name.strip_suffix(".ru.md") {
        dir.join(format!("{stem}.md"))
    } else if let Some(stem) = name.strip_suffix(".md") {
        dir.join(format!("{stem}.ru.md"))
    } else {
        path.to_path_buf()
    }
}

/// Print a failure report and exit non-zero when `errors` is not empty.
pub fn finish(tool: &str, errors: Vec<String>) {
    if errors.is_empty() {
        println!("{tool}: OK");
        return;
    }
    eprintln!("{tool}: {} problem(s)", errors.len());
    for error in &errors {
        eprintln!("  - {error}");
    }
    std::process::exit(1);
}
