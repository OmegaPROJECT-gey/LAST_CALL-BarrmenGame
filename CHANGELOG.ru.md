# Чейнджлог

Все значимые изменения LAST CALL на обоих языках. Разделы подписаны точной версией из [VERSIONING.ru.md](VERSIONING.ru.md); свежайший раздел — сверху. Этот файл — русский двойник [CHANGELOG.md](CHANGELOG.md), где записи ведутся на английском.

## Unreleased

- F0 в работе: скелет проекта UE 5.8 (`LastCall.uproject`, модули `Source/` runtime + editor, фазовые часы `ULCPhaseManager`, конфиги нуар-рендера/управления), дизайн-данные с JSON-схемами и три линтера — `lc-data-lint`, `lc-dialogue-lint` (граф + парность EN/RU), `lc-docs-check` (ссылки-двойники + целостность ссылок) — локально и в CI. Осталось: сборка движком на Linux/Windows (UE на машине ещё нет).
- Создан набор документации: README, PHILOSOPHY, ARCHITECTURE, ROADMAP, CHANGELOG, CONTRIBUTING, CODE_OF_CONDUCT, SECURITY, VERSIONING — каждый на английском и русском, с парными ссылками.
- Документы игры: [GAME-DESIGN](docs/GAME-DESIGN.ru.md), [NARRATIVE](docs/NARRATIVE.ru.md), [ART-BIBLE](docs/ART-BIBLE.ru.md), [LANGUAGES](docs/LANGUAGES.ru.md), [FREE-3D-MODELS](docs/FREE-3D-MODELS.ru.md), [MAINTENANCE](docs/MAINTENANCE.ru.md).
- Выбран стек: Unreal Engine 5 (5.8) + C++ для игры, Rust (Zig допустим) для инструментов пайплайна, Python для скриптов редактора (решение владельца, 13.10.2026, с цитируемыми источниками в [docs/LANGUAGES.ru.md](docs/LANGUAGES.ru.md)).
- Каталог бесплатных 3D-ассетов: ~90 моделей с проверенными URL и записанными лицензиями (CC0 / CC-BY / EULA платформ), плюс минимальный набор для грей-бокса (проверено 13.10.2026).
- Выбрана лицензия: **MIT** на код и документацию, контент игры остаётся проприетарным ([LICENSE](LICENSE), решение владельца 13.10.2026).
- Выбран язык текста игры: нативно на **обоих** языках с самого начала, с линтом парности поверх `Data/Localization/` (решение владельца 13.10.2026).
- Зафиксирован состав команды: соло-разработчик + AI-агенты; вехи прорабатываются последовательно и автономно.
- Определён режим ведения документации: двуязычная синхронизация, пересмотр на каждой сессии, дисциплина чейнджлога (см. [docs/MAINTENANCE.ru.md](docs/MAINTENANCE.ru.md)).
