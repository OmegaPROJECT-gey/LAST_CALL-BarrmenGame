//! `lc-dialogue-lint` - dialogue graph integrity plus EN/RU localization parity.
//!
//! Checks (ARCHITECTURE.md section 7, NARRATIVE.md):
//! - the `entry` node exists;
//! - every `next` (node and choice) points at an existing node;
//! - every node is reachable from `entry` (no dead scenes);
//! - every node terminates: `end`, a `next`, or choices with targets;
//! - every `text_key` exists in *both* `en.json` and `ru.json` - neither
//!   language is downstream of the other (owner decision 2026-10-13).

use std::collections::{BTreeMap, BTreeSet};
use std::path::PathBuf;

use serde_json::Value;

use lc_lint::{finish, repo_root};

type Loc = BTreeMap<String, String>;

fn load_localization(path: &PathBuf, errors: &mut Vec<String>) -> Loc {
    std::fs::read_to_string(path)
        .map_err(|e| format!("{}: unreadable: {e}", path.display()))
        .and_then(|text| {
            serde_json::from_str::<Value>(&text).map_err(|e| format!("{}: invalid JSON: {e}", path.display()))
        })
        .map(|value| {
            value
                .as_object()
                .expect("localization is an object")
                .iter()
                .filter_map(|(k, v)| v.as_str().map(|s| (k.clone(), s.to_string())))
                .collect()
        })
        .unwrap_or_else(|e| {
            errors.push(e);
            BTreeMap::new()
        })
}

fn main() {
    let root = repo_root();
    let mut errors: Vec<String> = Vec::new();

    let en = load_localization(&root.join("Data/Localization/en.json"), &mut errors);
    let ru = load_localization(&root.join("Data/Localization/ru.json"), &mut errors);

    // Parity of the localization tables themselves.
    for key in en.keys() {
        if !ru.contains_key(key) {
            errors.push(format!("localization: key `{key}` missing in ru.json"));
        }
    }
    for key in ru.keys() {
        if !en.contains_key(key) {
            errors.push(format!("localization: key `{key}` missing in en.json"));
        }
    }
    for (key, text) in &en {
        if text.trim().is_empty() {
            errors.push(format!("localization: key `{key}` is empty in en.json"));
        }
    }
    for (key, text) in &ru {
        if text.trim().is_empty() {
            errors.push(format!("localization: key `{key}` is empty in ru.json"));
        }
    }

    let dialogue_dir = root.join("Data/Dialogue");
    let mut files: Vec<PathBuf> = std::fs::read_dir(&dialogue_dir)
        .map(|it| {
            it.filter_map(|e| e.ok().map(|e| e.path()))
                .filter(|p| p.is_file() && p.extension().is_some_and(|e| e == "json"))
                .collect()
        })
        .unwrap_or_default();
    files.sort();

    if files.is_empty() {
        errors.push("Data/Dialogue/: no dialogue files".into());
    }

    for file in files {
        let rel = file
            .strip_prefix(&root)
            .unwrap_or(&file)
            .display()
            .to_string();
        let text = match std::fs::read_to_string(&file) {
            Ok(t) => t,
            Err(e) => {
                errors.push(format!("{rel}: unreadable: {e}"));
                continue;
            }
        };
        let doc: Value = match serde_json::from_str(&text) {
            Ok(v) => v,
            Err(e) => {
                errors.push(format!("{rel}: invalid JSON: {e}"));
                continue;
            }
        };
        let Some(nodes) = doc.get("nodes").and_then(Value::as_object) else {
            errors.push(format!("{rel}: no `nodes` object"));
            continue;
        };
        let entry = doc.get("entry").and_then(Value::as_str).unwrap_or("");

        if !nodes.contains_key(entry) {
            errors.push(format!("{rel}: entry node `{entry}` does not exist"));
        }

        // Edges and termination.
        let mut edges: BTreeMap<&str, Vec<&str>> = BTreeMap::new();
        let mut keys: BTreeSet<&str> = BTreeSet::new();

        for (id, node) in nodes {
            let mut targets: Vec<&str> = Vec::new();
            if let Some(next) = node.get("next").and_then(Value::as_str) {
                targets.push(next);
            }
            let mut terminates = node.get("end").and_then(Value::as_bool) == Some(true) || !targets.is_empty();
            if let Some(choices) = node.get("choices").and_then(Value::as_array) {
                terminates = terminates || !choices.is_empty();
                for choice in choices {
                    if let Some(text_key) = choice.get("text_key").and_then(Value::as_str) {
                        keys.insert(text_key);
                    }
                    match choice.get("next").and_then(Value::as_str) {
                        Some(next) => targets.push(next),
                        None => errors.push(format!("{rel}: node `{id}`: a choice has no `next`")),
                    }
                }
            }
            if let Some(text_key) = node.get("text_key").and_then(Value::as_str) {
                keys.insert(text_key);
            } else {
                errors.push(format!("{rel}: node `{id}` has no `text_key`"));
            }
            if !terminates {
                errors.push(format!("{rel}: node `{id}` dead-ends: no `end`, `next` or choices"));
            }
            for target in &targets {
                if !nodes.contains_key(*target) {
                    errors.push(format!("{rel}: node `{id}` -> unknown node `{target}`"));
                }
            }
            edges.insert(id.as_str(), targets);
        }

        // Reachability from entry.
        if nodes.contains_key(entry) {
            let mut seen: BTreeSet<&str> = BTreeSet::new();
            let mut stack = vec![entry];
            while let Some(id) = stack.pop() {
                if !seen.insert(id) {
                    continue;
                }
                if let Some(targets) = edges.get(id) {
                    stack.extend(targets.iter().copied());
                }
            }
            for id in nodes.keys() {
                if !seen.contains(id.as_str()) {
                    errors.push(format!("{rel}: node `{id}` unreachable from entry `{entry}`"));
                }
            }
        }

        // Text keys exist in both languages.
        for key in keys {
            if !en.contains_key(key) {
                errors.push(format!("{rel}: text_key `{key}` missing in en.json"));
            }
            if !ru.contains_key(key) {
                errors.push(format!("{rel}: text_key `{key}` missing in ru.json"));
            }
        }
    }

    finish("lc-dialogue-lint", errors);
}
