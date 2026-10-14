//! `lc-data-lint` - validates every design data file in `Data/` against its
//! JSON Schema in `Data/schemas/`.
//!
//! Directory -> schema mapping is explicit: a data directory without a schema
//! is an error, so new content types cannot sneak in unvalidated
//! (ARCHITECTURE.md section 7, docs/MAINTENANCE.md section 6 rule 5).

use std::collections::BTreeMap;
use std::path::{Path, PathBuf};

use jsonschema::Validator;
use serde_json::Value;

use lc_lint::{finish, repo_root};

fn schema_for(dir: &str) -> Option<&'static str> {
    match dir {
        "Recipes" => Some("recipe.schema.json"),
        "Customers" => Some("customer.schema.json"),
        "Dialogue" => Some("dialogue.schema.json"),
        "Skills" => Some("skills.schema.json"),
        "Liquids" => Some("liquids.schema.json"),
        "Economy" => Some("economy.schema.json"),
        "Crafting" => Some("crafting.schema.json"),
        "Factions" => Some("factions.schema.json"),
        "Interactions" => Some("interactions.schema.json"),
        "Night" => Some("night.schema.json"),
        "Balance" => Some("balance.schema.json"),
        "Localization" => Some("localization.schema.json"),
        _ => None,
    }
}

fn load(path: &Path) -> Result<Value, String> {
    let text = std::fs::read_to_string(path).map_err(|e| format!("{}: unreadable: {e}", path.display()))?;
    serde_json::from_str(&text).map_err(|e| format!("{}: invalid JSON: {e}", path.display()))
}

fn main() {
    let root = repo_root();
    let data = root.join("Data");
    let schemas = data.join("schemas");

    let mut errors: Vec<String> = Vec::new();
    let mut validators: BTreeMap<String, Validator> = BTreeMap::new();

    let entries = match std::fs::read_dir(&data) {
        Ok(e) => e,
        Err(e) => {
            finish("lc-data-lint", vec![format!("Data/ is unreadable: {e}")]);
            return;
        }
    };

    let mut data_dirs: Vec<PathBuf> = entries
        .filter_map(|e| e.ok().map(|e| e.path()))
        .filter(|p| p.is_dir() && p.file_name().is_some_and(|n| n != "schemas"))
        .collect();
    data_dirs.sort();

    for dir in data_dirs {
        let dir_name = dir.file_name().expect("dir name").to_string_lossy().into_owned();
        let Some(schema_file) = schema_for(&dir_name) else {
            errors.push(format!(
                "Data/{dir_name}/ has no schema mapping - add one to schema_for() and a JSON Schema to Data/schemas/"
            ));
            continue;
        };

        let validator = match validators.get(schema_file) {
            Some(v) => v,
            None => {
                let schema_path = schemas.join(schema_file);
                match load(&schema_path).and_then(|schema| {
                    Validator::new(&schema).map_err(|e| format!("{}: invalid schema: {e}", schema_path.display()))
                }) {
                    Ok(v) => validators.entry(schema_file.to_string()).or_insert(v),
                    Err(e) => {
                        errors.push(e);
                        continue;
                    }
                }
            }
        };

        let mut files: Vec<PathBuf> = std::fs::read_dir(&dir)
            .map(|it| {
                it.filter_map(|e| e.ok().map(|e| e.path()))
                    .filter(|p| p.is_file() && p.extension().is_some_and(|e| e == "json"))
                    .collect()
            })
            .unwrap_or_default();
        files.sort();

        if dir_name != "Localization" && files.is_empty() {
            errors.push(format!("Data/{dir_name}/: no data files"));
        }

        for file in files {
            let rel = file.strip_prefix(&root).unwrap_or(&file).display();
            let instance = match load(&file) {
                Ok(v) => v,
                Err(e) => {
                    errors.push(e);
                    continue;
                }
            };
            for err in validator.iter_errors(&instance) {
                let path = err.instance_path.to_string();
                errors.push(format!("{rel}: {path}: {err}"));
            }
        }
    }

    finish("lc-data-lint", errors);
}
