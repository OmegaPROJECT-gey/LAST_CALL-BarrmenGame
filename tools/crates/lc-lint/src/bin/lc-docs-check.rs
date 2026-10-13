//! `lc-docs-check` - bilingual documentation integrity (docs/MAINTENANCE.md).
//!
//! - every `.md` file has its language twin (`FOO.md` <-> `FOO.ru.md`);
//! - the twin is linked from the file itself;
//! - every relative link resolves to a file (code blocks are ignored so the
//!   examples inside docs/MAINTENANCE.md do not count as links).

use std::path::{Path, PathBuf};

use lc_lint::{extract_links, finish, markdown_files, repo_root, strip_code, twin_of};

fn main() {
    let root = repo_root();
    let files = markdown_files(&root);
    let mut errors: Vec<String> = Vec::new();
    let mut visited_pairs: std::collections::HashSet<PathBuf> = Default::default();

    if files.is_empty() {
        finish("lc-docs-check", vec!["no markdown files found".into()]);
        return;
    }

    for file in &files {
        // Each language pair is checked once, not twice (both files are listed).
        let file_name = file.file_name().expect("file name").to_string_lossy().into_owned();
        let pair_key = if let Some(stem) = file_name.strip_suffix(".ru.md") {
            file.parent().unwrap_or(std::path::Path::new(".")).join(format!("{stem}.md"))
        } else {
            file.clone()
        };
        if !visited_pairs.insert(pair_key) {
            continue;
        }
        let rel = file.strip_prefix(&root).unwrap_or(file).display().to_string();
        let raw = match std::fs::read_to_string(file) {
            Ok(t) => t,
            Err(e) => {
                errors.push(format!("{rel}: unreadable: {e}"));
                continue;
            }
        };

        // 1. The language twin exists.
        let twin: PathBuf = twin_of(file);
        if !twin.exists() {
            let twin_name = twin.file_name().expect("twin name").to_string_lossy();
            errors.push(format!("{rel}: missing language twin {twin_name}"));
            continue;
        }
        let twin_rel = twin.strip_prefix(&root).unwrap_or(&twin).display().to_string();

        // 2. The twin is linked from this file (pair link at the top).
        let twin_name = twin.file_name().expect("twin name").to_string_lossy();
        if !raw.contains(&format!("]({twin_name})")) {
            errors.push(format!("{rel}: no link to its twin {twin_name}"));
        }

        // 3. Every relative link resolves, from either language of the pair.
        for (path, text) in [(file, &raw), (&twin, &std::fs::read_to_string(&twin).unwrap_or_default())] {
            let path_rel = path.strip_prefix(&root).unwrap_or(path).display().to_string();
            for target in extract_links(&strip_code(text)) {
                if target.starts_with("http://")
                    || target.starts_with("https://")
                    || target.starts_with("mailto:")
                    || target.starts_with('#')
                {
                    continue;
                }
                let clean = target.split('#').next().unwrap_or("").trim();
                if clean.is_empty() {
                    continue;
                }
                let resolved: PathBuf = path
                    .parent()
                    .unwrap_or(Path::new("."))
                    .join(clean)
                    .canonicalize()
                    .map(|p| p)
                    .unwrap_or_else(|_| path.parent().unwrap_or(Path::new(".")).join(clean));
                if !resolved.exists() {
                    errors.push(format!("{path_rel}: broken link `{target}`"));
                }
            }
        }

        // Silence unused warning for twin_rel in the happy path.
        let _ = twin_rel;
    }

    finish("lc-docs-check", errors);
}
